#include "test/TestUtils.hpp"

int flag;
int log_num;
TableId table_id_array[20];

int compare_tik(const void* first, const void* second) {
	auto* left = (TableIdKey*)first;
	auto* right = (TableIdKey*)second;

	if (left->table_id < right->table_id) return -1;
	if (left->table_id > right->table_id) return 1;
	if (left->key < right->key) return -1;
	if (left->key > right->key) return 1;
	return 0;
}

void sort_table_id_key(TableId table_ids[], Key keys[], int count) {
	auto* tik = (TableIdKey*)malloc(sizeof(TableIdKey) * count);

	// length of array >= count * 2
	for (int i = 0; i < count; i++) {
		tik[i].table_id = table_ids[i];
		tik[i].key = keys[i];
	}

	qsort(tik, count, sizeof(TableIdKey), compare_tik);

	for (int i = 0; i < count; i++) {
		table_ids[i] = tik[i].table_id;
		keys[i] = tik[i].key;
	}

	free(tik);
}