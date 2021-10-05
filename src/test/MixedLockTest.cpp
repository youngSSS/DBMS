#include "test/Test.hpp"

#define MLT_TABLE_NUMBER       (1)
#define MLT_TABLE_SIZE         (100)
#define MLT_THREAD_NUMBER      (10)

#define MLT_FIND_UPDATE_NUMBER (20)
#define MLT_OPERATION_NUMBER   (100000)

pthread_mutex_t MLT_mutex;
int64_t MLT_operation_count;

void* MLT_func(void* args) {
	int64_t operation_count;
	TableId table_ids[MLT_FIND_UPDATE_NUMBER];
	Key keys[MLT_FIND_UPDATE_NUMBER];
	Value val;
	Value ret_val;
	TransactionId transaction_id;
	int ret;

	for (;;) {
		pthread_mutex_lock(&MLT_mutex);
		operation_count = MLT_operation_count++;
		pthread_mutex_unlock(&MLT_mutex);
		if (operation_count > MLT_OPERATION_NUMBER)
			break;

		for (int i = 0; i < MLT_FIND_UPDATE_NUMBER; i++) {
			table_ids[i] = table_id_array[rand() % MLT_TABLE_NUMBER];
			keys[i] = rand() % MLT_TABLE_SIZE;
		}
		/* sorting for avoiding deadlock */
		sort_table_id_key(table_ids, keys, MLT_FIND_UPDATE_NUMBER);

		/* transaction begin */
		transaction_id = trx_begin();

		for (int i = 0; i < MLT_FIND_UPDATE_NUMBER; i++) {
			if (i != 0 && table_ids[i] == table_ids[i - 1] && keys[i] == keys[i - 1])
				/* Avoid accessing the same record twice. */
				continue;

			if (rand() % 2 == 0) {
				/* db_find */
				ret = db_find(table_ids[i], keys[i], ret_val, transaction_id);
				if (ret != 0) {
					printf("INCORRECT: fail to db_find()\n");
					return nullptr;
				}
				if (keys[i] != 0 && (atoi(ret_val) % keys[i]) != 0) {
					printf("INCORRECT: value is wrong\n");
					return nullptr;
				}
			}
			else {
				/* db_update */
				sprintf(val, "%ld", keys[i] * (rand() % 100));
				ret = db_update(table_ids[i], keys[i], val, transaction_id);
				if (ret != 0) {
					return nullptr;
				}
			}

		}

//        /* transaction commit */
//        if (transaction_id % 2 == 0)
		trx_commit(transaction_id);
	}

	return nullptr;
}

void mlock_test() {
	pthread_t threads[MLT_THREAD_NUMBER];
	int64_t operation_count_0;
	int64_t operation_count_1;

	/* Initiate variables for test. */
	MLT_operation_count = 0;
	pthread_mutex_init(&MLT_mutex, nullptr);

	/* Initiate database. */
	init_db(10, flag, log_num, "LogFile.db", "LogMessageFile.txt");

	/* open table */
	for (int i = 0; i < MLT_TABLE_NUMBER; i++) {
		char* str = (char*)malloc(sizeof(char) * 100);
		TableId table_id;
		sprintf(str, "DATA%d", i + 1);
		table_id = open_table(str);
		table_id_array[i] = table_id;

		/* insertion */
		for (Key key = 0; key < MLT_TABLE_SIZE; key++) {
			Value value;
			sprintf(value, "%ld", (Key)key);
			db_insert(table_id, key, value);
		}
	}

	printf("database init\n");

	/* thread create */
	for (int i = 0; i < MLT_THREAD_NUMBER; i++) {
		pthread_create(&threads[i], 0, MLT_func, nullptr);
	}

	for (;;) {
		pthread_mutex_lock(&MLT_mutex);
		operation_count_0 = MLT_operation_count;
		pthread_mutex_unlock(&MLT_mutex);
		if (operation_count_0 > MLT_OPERATION_NUMBER)
			break;

		sleep(1);

		pthread_mutex_lock(&MLT_mutex);
		operation_count_1 = MLT_operation_count;
		pthread_mutex_unlock(&MLT_mutex);
		if (operation_count_1 > MLT_OPERATION_NUMBER)
			break;

		if (operation_count_0 == operation_count_1) {
			printf("INCORRECT: all threads are working nothing.\n");
			print_lock_table();
		}
	}

	/* thread join */
	for (int i = 0; i < MLT_THREAD_NUMBER; i++) {
		pthread_join(threads[i], nullptr);
	}

	/* close table */
	for (int i = 0; i < MLT_TABLE_NUMBER; i++) {
		TableId table_id;
		table_id = table_id_array[i];
		close_table(table_id);
	}

	/* shutdown db */
	shutdown_db();
}