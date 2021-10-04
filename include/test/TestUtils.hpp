#ifndef DBMS_INCLUDE_TESTUTILS_HPP_
#define DBMS_INCLUDE_TESTUTILS_HPP_

#define DATABASE_BUFFER_SIZE (100)

#include <cstdlib>

typedef int TableId;
typedef int64_t Key;
typedef int TransactionId;
typedef char Value[120];

typedef struct {
	TableId table_id;
	Key key;
} TableIdKey;

extern int flag;
extern int log_num;
extern TableId table_id_array[20];

// compare pairs of table id & key
int compare_tik(const void* first, const void* second);

// sort pairs of table id & key
void sort_table_id_key(TableId table_ids[], Key keys[], int count);

#endif //DBMS_INCLUDE_TESTUTILS_HPP_
