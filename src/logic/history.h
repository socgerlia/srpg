#pragma once

class op_result{
};

class record{
	int time;
	int entity_id;
	operation* op;
	op_result* result;
};

class history{
	struct i_record{};
	typedef multi_index_container<
		record,
		indexed_by<
			ordered_non_unique<tag<i_record>, member<record, int, &record::time>>
		>
	> record_set;
	record_set records;
};
