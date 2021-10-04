#include "test/Test.hpp"

#define DLT_TABLE_NUMBER        (1)
#define DLT_TABLE_SIZE          (100)
#define DLT_THREAD_NUMBER       (5)

#define DLT_FIND_UPDATE_NUMBER  (5)
#define DLT_OPERATION_NUMBER    (100000)

pthread_mutex_t DLT_mutex;
int64_t DLT_operation_count;

void* DLT_func(void* args) {
	int64_t operation_count;
	TableId table_ids[DLT_FIND_UPDATE_NUMBER];
	Key keys[DLT_FIND_UPDATE_NUMBER];
	Value val;
	Value ret_val;
	TransactionId transaction_id;
	int ret;
	bool flag;

	for (;;) {
		pthread_mutex_lock(&DLT_mutex);
		operation_count = DLT_operation_count++;
		pthread_mutex_unlock(&DLT_mutex);
		if (operation_count > DLT_OPERATION_NUMBER)
			break;

		for (int i = 0; i < DLT_FIND_UPDATE_NUMBER; i++) {
			table_ids[i] = table_id_array[rand() % DLT_TABLE_NUMBER];
			keys[i] = rand() % DLT_TABLE_SIZE;
		}

		/* transaction begin */
		transaction_id = trx_begin();

		for (int i = 0; i < DLT_FIND_UPDATE_NUMBER; i++) {
			flag = false;
			for (int j = 0; j < i; j++) {
				if (table_ids[i] == table_ids[j] && keys[i] == keys[j]) {
					flag = true;
				}
			}
			if (flag == true)
				/* avoid accessing same record twice */
				continue;

			if (rand() % 2 == 0) {
				/* db_find */
				ret = db_find(table_ids[i], keys[i], ret_val, transaction_id);
				if (ret != 0) {
					/* abort */
					break;
				}
				if (keys[i] != 0 && (atoi(ret_val) % keys[i]) != 0) {
					printf("INCORRECT: value is wrong\n");
					return NULL;
				}
			}
			else {
				/* db_update */
				sprintf(val, "%ld", keys[i] * (rand() % 100));
				ret = db_update(table_ids[i], keys[i], val, transaction_id);
				if (ret != 0) {
					/* abort */
					break;
				}
			}
		}

		trx_commit(transaction_id);
	}

	return NULL;
}

void deadlock_test() {
	pthread_t threads[DLT_THREAD_NUMBER];
	int64_t operation_count_0;
	int64_t operation_count_1;

	/* Initiate variables for test. */
	DLT_operation_count = 0;
	pthread_mutex_init(&DLT_mutex, NULL);

	/* Initiate database. */
	init_db(DATABASE_BUFFER_SIZE, flag, log_num, "LogFile.db", "LogMessageFile.txt");

	/* open table */
	for (int i = 0; i < DLT_TABLE_NUMBER; i++) {
		char* str = (char*)malloc(sizeof(char) * 100);
		TableId table_id;
		sprintf(str, "DATA%d", i + 1);
		table_id = open_table(str);
		table_id_array[i] = table_id;

		/* insertion */
		for (Key key = 0; key < DLT_TABLE_SIZE; key++) {
			Value value;
			sprintf(value, "%ld", (Key)key);
			db_insert(table_id, key, value);
		}
	}

	printf("database init\n");

	/* thread create */
	for (int i = 0; i < DLT_THREAD_NUMBER; i++) {
		pthread_create(&threads[i], 0, DLT_func, NULL);
	}

	for (;;) {
		pthread_mutex_lock(&DLT_mutex);
		operation_count_0 = DLT_operation_count;
		pthread_mutex_unlock(&DLT_mutex);
		if (operation_count_0 > DLT_OPERATION_NUMBER)
			break;

		sleep(1);

		pthread_mutex_lock(&DLT_mutex);
		operation_count_1 = DLT_operation_count;
		pthread_mutex_unlock(&DLT_mutex);
		if (operation_count_1 > DLT_OPERATION_NUMBER)
			break;

		if (operation_count_0 == operation_count_1) {
			printf("INCORRECT: all threads are working nothing.\n");
			//print_lock_table();
		}
	}

	/* thread join */
	for (int i = 0; i < DLT_THREAD_NUMBER; i++) {
		pthread_join(threads[i], NULL);
	}

	/* close table */
	for (int i = 0; i < DLT_TABLE_NUMBER; i++) {
		TableId table_id;
		table_id = table_id_array[i];
		close_table(table_id);
	}

	/* shutdown db */
	shutdown_db();
}