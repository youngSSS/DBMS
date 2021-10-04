#include "test/Test.hpp"

#define XLT_TABLE_NUMBER     (1)
#define XLT_TABLE_SIZE       (100)
#define XLT_THREAD_NUMBER    (10)

#define XLT_UPDATE_NUMBER    (10)
#define XLT_OPERATION_NUMBER (100000)

pthread_mutex_t XLT_mutex;
int64_t XLT_operation_count;

void* XLT_func(void* args) {
	int64_t operation_count;
	TableId table_ids[XLT_UPDATE_NUMBER];
	Key keys[XLT_UPDATE_NUMBER];
	Value val;
	TransactionId transaction_id;
	int ret;

	for (;;) {
		pthread_mutex_lock(&XLT_mutex);
		operation_count = XLT_operation_count++;
		pthread_mutex_unlock(&XLT_mutex);
		if (operation_count > XLT_OPERATION_NUMBER)
			break;

		for (int i = 0; i < XLT_UPDATE_NUMBER; i++) {
			table_ids[i] = table_id_array[rand() % XLT_TABLE_NUMBER];
			keys[i] = rand() % XLT_TABLE_SIZE;
		}
		/* sorting for avoiding deadlock */
		sort_table_id_key(table_ids, keys, XLT_UPDATE_NUMBER);

		/* transaction begin */
		transaction_id = trx_begin();

		for (int i = 0; i < XLT_UPDATE_NUMBER; i++) {
			if (i != 0 && table_ids[i] == table_ids[i - 1] && keys[i] == keys[i - 1])
				/* Avoid accessing the same record twice. */
				continue;

			sprintf(val, "%ld", keys[i]);
			ret = db_update(table_ids[i], keys[i], val, transaction_id);
			if (ret != 0) {
				printf("INCORRECT: fail to db_update()\n"
					   "table id : %d, key : %d, value : %d, Trx_id :%d\n", table_ids[i], keys[i], val, transaction_id
				);
				return NULL;
			}
		}

		if (transaction_id % 2 == 0) {
			printf("COMMIT : %d\n", transaction_id);
			trx_commit(transaction_id);
		}

	}

	return NULL;
}

void xlock_test() {
	pthread_t threads[XLT_THREAD_NUMBER];
	int64_t operation_count_0;
	int64_t operation_count_1;

	/* Initiate variables for test. */
	XLT_operation_count = 0;
	pthread_mutex_init(&XLT_mutex, NULL);

	/* Initiate database. */
	init_db(DATABASE_BUFFER_SIZE, flag, log_num, "LogFile.db", "LogMessageFile.txt");

	/* open table */
	for (int i = 0; i < XLT_TABLE_NUMBER; i++) {
		char* str = (char*)malloc(sizeof(char) * 100);
		TableId table_id;
		sprintf(str, "DATA%d", i + 1);
		table_id = open_table(str);
		table_id_array[i] = table_id;

		/* insertion */
		for (Key key = 0; key < XLT_TABLE_SIZE; key++) {
			Value value;
			sprintf(value, "%ld", (Key)0);
			db_insert(table_id, key, value);
		}
	}

	printf("database init\n");

	/* thread create */
	for (int i = 0; i < XLT_THREAD_NUMBER; i++) {
		pthread_create(&threads[i], 0, XLT_func, NULL);
	}

	for (;;) {
		pthread_mutex_lock(&XLT_mutex);
		operation_count_0 = XLT_operation_count;
		pthread_mutex_unlock(&XLT_mutex);
		if (operation_count_0 > XLT_OPERATION_NUMBER)
			break;

		sleep(1);

		pthread_mutex_lock(&XLT_mutex);
		operation_count_1 = XLT_operation_count;
		pthread_mutex_unlock(&XLT_mutex);
		if (operation_count_1 > XLT_OPERATION_NUMBER)
			break;

		if (operation_count_0 == operation_count_1) {
			printf("INCORRECT: all threads are working nothing.\n");
			//print_lock_table();
		}
	}

	/* thread join */
	for (int i = 0; i < XLT_THREAD_NUMBER; i++) {
		pthread_join(threads[i], NULL);
	}

	/* close table */
	for (int i = 0; i < XLT_TABLE_NUMBER; i++) {
		TableId table_id;
		table_id = table_id_array[i];
		close_table(table_id);
	}

	/* shutdown db */
	shutdown_db();
}