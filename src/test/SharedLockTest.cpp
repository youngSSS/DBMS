#include "test/Test.hpp"

#define SLT_TABLE_NUMBER     (1)
#define SLT_TABLE_SIZE       (100)
#define SLT_THREAD_NUMBER    (10)

#define SLT_FIND_NUMBER      (10)
#define SLT_OPERATION_NUMBER (100000)

pthread_mutex_t SLT_mutex;
int64_t SLT_operation_count;

void* SLT_func(void* args) {
	int64_t operation_count;
	TableId table_ids[SLT_FIND_NUMBER];
	Key keys[SLT_FIND_NUMBER];
	Value ret_val;
	TransactionId transaction_id;
	int ret;

	for (;;) {
		pthread_mutex_lock(&SLT_mutex);
		operation_count = SLT_operation_count++;
		pthread_mutex_unlock(&SLT_mutex);
		if (operation_count > SLT_OPERATION_NUMBER)
			break;

		for (int i = 0; i < SLT_FIND_NUMBER; i++) {
			table_ids[i] = table_id_array[rand() % SLT_TABLE_NUMBER];
			keys[i] = rand() % SLT_TABLE_SIZE;
		}
		sort_table_id_key(table_ids, keys, SLT_FIND_NUMBER);

		/* transaction begin */
		transaction_id = trx_begin();

		for (int i = 0; i < SLT_FIND_NUMBER; i++) {
			if (i != 0 && table_ids[i] == table_ids[i - 1] && keys[i] == keys[i - 1])
				/* Avoid accessing the same record twice. */
				continue;

			ret = db_find(table_ids[i], keys[i], ret_val, transaction_id);
			if (ret != 0) {
				printf("INCORRECT: fail to db_find()\n", pthread_self());
				return nullptr;
			}
			if (atoi(ret_val) != keys[i]) {
				printf("INCORRECT: value is wrong\n");
				printf("value : %d, ret_val : %d\n", pthread_self(), i, keys[i], atoi(ret_val));
				return nullptr;
			}
		}

		/* transaction commit */
		trx_commit(transaction_id);
	}

	return nullptr;
}

void slock_test() {
	pthread_t threads[SLT_THREAD_NUMBER];
	int64_t operation_count_0;
	int64_t operation_count_1;

	/* Initiate variables for test. */
	SLT_operation_count = 0;
	pthread_mutex_init(&SLT_mutex, nullptr);

	/* Initiate database. */
	init_db(DATABASE_BUFFER_SIZE, flag, log_num, "LogFile.db", "LogMessageFile.txt");

	/* open table */
	for (int i = 0; i < SLT_TABLE_NUMBER; i++) {
		char* str = (char*)malloc(sizeof(char) * 100);
		TableId table_id;
		sprintf(str, "DATA%d", i + 1);
		table_id = open_table(str);
		table_id_array[i] = table_id;

		/* insertion */
		for (Key key = 0; key < SLT_TABLE_SIZE; key++) {
			Value value;
			sprintf(value, "%ld", key);
			db_insert(table_id, key, value);
		}
	}

	printf("database init\n");

	/* thread create */
	for (int i = 0; i < SLT_THREAD_NUMBER; i++) {
		pthread_create(&threads[i], 0, SLT_func, nullptr);
	}

	for (;;) {
		pthread_mutex_lock(&SLT_mutex);
		operation_count_0 = SLT_operation_count;
		pthread_mutex_unlock(&SLT_mutex);
		if (operation_count_0 > SLT_OPERATION_NUMBER)
			break;

		sleep(1);

		pthread_mutex_lock(&SLT_mutex);
		operation_count_1 = SLT_operation_count;
		pthread_mutex_unlock(&SLT_mutex);
		if (operation_count_1 > SLT_OPERATION_NUMBER)
			break;

		if (operation_count_0 == operation_count_1) {
			printf("INCORRECT: all threads are working nothing.\n");
		}
	}

	/* thread join */
	for (int i = 0; i < SLT_THREAD_NUMBER; i++) {
		pthread_join(threads[i], nullptr);
	}

	/* close table */
	for (int i = 0; i < SLT_TABLE_NUMBER; i++) {
		TableId table_id;
		table_id = table_id_array[i];
		close_table(table_id);
	}

	/* shutdown db */
	shutdown_db();
}