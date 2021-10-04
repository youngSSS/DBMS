#include "test/Test.hpp"

#define SST_TABLE_NUMBER     (1)
#define SST_TABLE_SIZE       (100)
#define SST_OPERATION_NUMBER (100)

pthread_mutex_t SST_mutex;
int64_t SST_operation_count;

void* SST_func(void* args) {
	int64_t operation_count;
	TableId table_id;
	Key key1, key2;
	Value value;
	Value ret_val;
	TransactionId transaction_id;
	int ret;

	for (;;) {
		pthread_mutex_lock(&SST_mutex);
		operation_count = SST_operation_count++;
		pthread_mutex_unlock(&SST_mutex);
		if (operation_count > SST_OPERATION_NUMBER)
			break;

		table_id = table_id_array[rand() % SST_TABLE_NUMBER];
		key1 = rand() % SST_TABLE_SIZE;
		key2 = rand() % SST_TABLE_SIZE;
		sprintf(value, "%ld", key2);

		if (key1 == key2)
			/* Avoid accessing the same record twice. */
			continue;

		transaction_id = trx_begin();

		ret = db_find(table_id, key1, ret_val, transaction_id);
		if (ret != 0) {
			printf("INCORRECT: fail to db_find()\n");
			return NULL;
		}
		if (atoi(ret_val) != 0 && atoi(ret_val) != key1) {
			printf("INCORRECT: value is wrong\n");
			return NULL;
		}

		ret = db_update(table_id, key2, value, transaction_id);
		if (ret != 0) {
			printf("INCORRECT: fail to db_update()\n");
			return NULL;
		}

		trx_commit(transaction_id);
	}

	return NULL;
}

/* simple single thread test */
void single_thread_test() {
	pthread_t thread;
	int64_t operation_count_0;
	int64_t operation_count_1;

	/* Initiate variables for test. */
	SST_operation_count = 0;
	pthread_mutex_init(&SST_mutex, NULL);

	/* Initiate database. */
	init_db(DATABASE_BUFFER_SIZE, flag, log_num, "LogFile.db", "LogMessageFile.txt");

	/* open table */
	for (int i = 0; i < SST_TABLE_NUMBER; i++) {
		char* str = (char*)malloc(sizeof(char) * 100);
		TableId table_id;
		sprintf(str, "DATA%d", i + 1);
		table_id = open_table(str);
		table_id_array[i] = table_id;

		/* insertion */
		for (Key key = 0; key < SST_TABLE_SIZE; key++) {
			Value value;
			sprintf(value, "%d", 0);
			db_insert(table_id, key, value);
		}
	}

	printf("database init\n");

	/* thread create */
	pthread_create(&thread, 0, SST_func, NULL);

	for (;;) {
		pthread_mutex_lock(&SST_mutex);
		operation_count_0 = SST_operation_count;
		pthread_mutex_unlock(&SST_mutex);
		if (operation_count_0 > SST_OPERATION_NUMBER)
			break;

		sleep(1);

		pthread_mutex_lock(&SST_mutex);
		operation_count_1 = SST_operation_count;
		pthread_mutex_unlock(&SST_mutex);
		if (operation_count_1 > SST_OPERATION_NUMBER)
			break;

		if (operation_count_0 == operation_count_1) {
			printf("INCORRECT: all threads are working nothing.\n");
		}
	}

	/* thread join */
	pthread_join(thread, NULL);

	/* close table */
	for (int i = 0; i < SST_TABLE_NUMBER; i++) {
		TableId table_id;
		table_id = table_id_array[i];
		close_table(table_id);
	}

	print_log();

	/* shutdown db */
	shutdown_db();
}