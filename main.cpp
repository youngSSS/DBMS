#include "Api.hpp"
#include "TransactionLayer.hpp"
#include "LogLayer.hpp"
#include "IndexLayer.hpp"
#include "test/Test.hpp"

#define DATABASE_BUFFER_SIZE (100)

int main() {

	// Command
	char instruction;

	// Data file
	char pathname[21];
	int table_id;

	// Buffer
	int buf_size;

	// Record (key-value pair)
	int64_t input_key;
	char input_value[120];

	// Return value
	char ret_val[120];

	// For command 'I', 'D'
	char a[120];
	int64_t in_start, in_end, del_start, del_end;

	// For time checking
	time_t start, end;

	// Result of each operation
	int result;

	// Transaction
	int mode;

	// Recovery
	int _flag = 0;
	int _log_num = 0;
	char _log_path[120];
	char _logmsg_path[120];
	int test_cnt = 0;

	/************************** VARIABLES **************************/
	// Initialize DB
	int BUF_SIZE = 100;
	int FLAG = 0;
	int LOG_NUM = 0;
	char LOG_PATH[20] = { "LOG_FILE.db" };
	char LOGMSG_PATH[20] = { "LOG_MSG.txt" };

	// Open data file
	char DATA_PATH[20] = { "DATA1" };
	int TABLE_ID;

	// Insert values
	char VALUE[120];

	// Transaction operations
	int TRX_ID[10];
	int UPDATE_CNT = 5;
	int UPDATE_KEY;
	char UPDATE_VALUE[120] = { "UPDATE" };
	char UPDATE_VALUE_1[120];
	char UPDATE_VALUE_2[120];
	/***************************************************************/

	// Print usage
	print_usage();
	printf("> ");

	while (scanf("%c", &instruction) != EOF) {

		switch (instruction) {

		case 'O':
			scanf("%s", pathname);

			table_id = open_table(pathname);

			if (table_id == -1)
				printf("Fail to open file\nFile open fault\n");
			else if (table_id == -2)
				printf("File name format is wrong\nFile name should be \"DATA00\"\nFile open fault\n");
			else
				printf("File open is completed\nTable id : %d\n", table_id);

			break;

		case 'B':
			scanf("%d", &buf_size);
			scanf("%d %d %s %s", &_flag, &_log_num, _log_path, _logmsg_path);
			result = init_db(buf_size, _flag, _log_num, _log_path, _logmsg_path);

			if (result == 0) printf("DB initializing is completed\n");
			else if (result == 1) printf("Buffer creation fault\n");
			else if (result == 2) printf("DB is already initialized\nDB initializing fault\n");
			else if (result == 3) printf("Buffer size must be over 0\nDB initializing fault\n");
			else if (result == 4) printf("Lock table initialize error\n");
			else if (result == 5) printf("Transaction mutex error\n");
			else if (result == 6) printf("Recovery error\n");
			else printf("? Error ?\n");

			break;

		case 'R':
			scanf("%d %d %s %s", &_flag, &_log_num, _log_path, _logmsg_path);

			index_init_db(DATABASE_BUFFER_SIZE);
			init_log(_logmsg_path);

			DB_recovery(_flag, _log_num, _log_path);

			break;

		case 'i':
			scanf("%d %ld %[^\n]", &table_id, &input_key, input_value);

			start = clock();
			result = db_insert(table_id, input_key, input_value);
			end = clock();

			if (result == 0) {
				printf("Insertion is completed\n");
				printf("Time : %f\n", (double)(end - start));
			}
			else if (result == 1) printf("Table_id[%d] file is not exist\n", table_id);
			else if (result == 2) printf("Duplicate key <%ld>\nInsertion fault\n", input_key);

			break;

		case 'f':
			scanf("%d %ld %[^\n]", &table_id, &input_key);

			start = clock();
			result = _find(table_id, input_key, input_value);
			end = clock();

			if (result == 0) {
				printf("Find is completed\n");
				printf("key: %lld, value: %d\n", input_key, input_value);
				printf("Time : %f\n", (double)(end - start));
			}
			else if (result == 2) printf("%lld is not exist in %d table\n", input_key, table_id);

			break;

		case 'd':
			scanf("%d %ld", &table_id, &input_key);
			start = clock();
			result = db_delete(table_id, input_key);
			end = clock();
			if (result == 0) {
				printf("Deletion is completed\n");
				printf("Time : %f\n", (double)(end - start));
			}
			else if (result == 1) printf("Table_id[%d] file is not exist\n", table_id);
			else if (result == 2) printf("No such key <%ld>\nDeletion fault\n", input_key);
			break;

		case 'u':
			/***************************** TEST ****************************/
			// Initialize DB
			init_db(BUF_SIZE, FLAG, LOG_NUM, LOG_PATH, LOGMSG_PATH);
			printf("SUCCESS :: INITIALIZE DB\n");

			// Open data file
			TABLE_ID = open_table(DATA_PATH);
			printf("SUCCESS :: OPEN DATA FILE\n");

			// Insert values
			for (int64_t i = 0; i <= 1000; i++) {
				sprintf(VALUE, "%ld", i);
				db_insert(TABLE_ID, i, VALUE);
			}
			printf("SUCCESS :: INSERT VALUES\n");

			// Transaction operations
			for (int i = 0; i < 10; i++) {

				TRX_ID[i] = trx_begin();

				for (int j = 0; j < UPDATE_CNT; j++) {
					UPDATE_KEY = rand() % 1000 + 1;
					strcpy(UPDATE_VALUE_1, UPDATE_VALUE);
					sprintf(UPDATE_VALUE_2, "%d", UPDATE_KEY);
					strcat(UPDATE_VALUE_1, UPDATE_VALUE_2);
					db_update(TABLE_ID, UPDATE_KEY, UPDATE_VALUE_1, TRX_ID[i]);
				}

				// COMMIT Transaction 1, 3, 5, 7
				if (TRX_ID[i] == 1 || TRX_ID[i] == 3 || TRX_ID[i] == 5 || TRX_ID[i] == 7)
					trx_commit(TRX_ID[i]);

					// ABORT Transaction 2, 4, 6, 10
				else if (TRX_ID[i] == 2 || TRX_ID[i] == 4 || TRX_ID[i] == 6 || TRX_ID[i] == 10)
					trx_abort(TRX_ID[i]);

					// Do not COMMIT OR ABORT Transaction 8, 9
				else
					continue;

				// FLUSH DATA BUFFER TO DATA DISK
				if (TRX_ID[i] == 4)
					db_flush(TABLE_ID);

			}
			printf("SUCCESS :: TEST\n");
			/***************************************************************/
			break;

		case 'L':
			write_log(0, 0);
			break;

		case 'I':
			scanf("%d %ld %ld", &table_id, &in_start, &in_end);
			strcpy(a, "a");
			start = clock();
			for (int64_t i = in_start; i <= in_end; i++) {
				sprintf(a, "%ld", i);
				result = db_insert(table_id, i, a);
				if (result == 2) printf("Duplicate key <%ld>\nInsertion fault\n", i);
			}
			end = clock();
			printf("Time : %f\n", (double)(end - start));
			break;

		case 'D':
			scanf("%d %ld %ld", &table_id, &del_start, &del_end);
			start = clock();
			for (int64_t i = del_start; i <= del_end; i++) {
				result = db_delete(table_id, i);
				if (result == 2) printf("No such key <%ld>\nDeletion fault\n", i);
			}
			end = clock();
			printf("Time : %f\n", (double)(end - start));
			break;

		case 'T':
			srand(123);

			scanf("%d", &mode);

			if (mode == 1) {
				single_thread_test();
			}
			else if (mode == 2) {
				slock_test();
			}
			else if (mode == 3) {
				xlock_test();
			}
			else if (mode == 4) {
				mlock_test();
			}
			else if (mode == 5) {
				deadlock_test();
			}
			break;

		case 't':
			db_print_table_list();
			break;

		case 'p':
			scanf("%d", &table_id);
			db_print(table_id);
			break;

		case 'l':
			scanf("%d", &table_id);
			db_print_leaf(table_id);
			break;

		case 'C':
			scanf("%d", &table_id);
			result = close_table(table_id);
			if (result == 0) printf("Close is completed\n");
			else if (result == 1) printf("File having table_id[%d] is not exist\nClose fault\n", table_id);
			else printf("Close fault\n");
			break;

		case 'S':
			result = shutdown_db();
			if (result == 0) printf("Shutdown is completed\n");
			else if (result == 1) printf("Buffer is not exist\nShutdown is completed\n");
			else printf("Shutdown fault\n");
			break;

		case 'Q':
			while (getchar() != (int)'\n');
			result = shutdown_db();
			if (result == 0) printf("Shutdown is completed\n");
			else if (result == 1) printf("Buffer is not exist\nShutdown is completed\n");
			else printf("Shutdown fault\n");
			return EXIT_SUCCESS;

		case 'F':
			scanf("%d", &table_id);
			db_flush(table_id);
			break;

		case 'U':
			print_usage();
			break;

		default:
			printf("Invalid Command\n");
			break;
		}

		while (getchar() != (int)'\n');
		printf("> ");

	}

	printf("\n");

	return EXIT_SUCCESS;
}
