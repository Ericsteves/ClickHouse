#include <DB/Columns/ColumnString.h>
#include <DB/DataTypes/DataTypeString.h>
#include <DB/DataTypes/DataTypesNumberFixed.h>
#include <DB/DataStreams/OneBlockInputStream.h>
#include <DB/Storages/StorageSystemSettings.h>


namespace DB
{


StorageSystemSettings::StorageSystemSettings(const std::string & name_, const Context & context_)
	: name(name_), context(context_)
	, columns{
		{ "name", 			new DataTypeString	},
		{ "value",			new DataTypeString	},
		{ "changed", 		new DataTypeUInt8	},
	}
{
}

StoragePtr StorageSystemSettings::create(const std::string & name_, const Context & context_)
{
	return (new StorageSystemSettings(name_, context_))->thisPtr();
}


BlockInputStreams StorageSystemSettings::read(
	const Names & column_names, ASTPtr query, const Settings & settings,
	QueryProcessingStage::Enum & processed_stage, size_t max_block_size, unsigned threads)
{
	check(column_names);
	processed_stage = QueryProcessingStage::FetchColumns;

	ColumnWithNameAndType col_name{new ColumnString, new DataTypeString, "name"};
	ColumnWithNameAndType col_value{new ColumnString, new DataTypeString, "value"};
	ColumnWithNameAndType col_changed{new ColumnUInt8, new DataTypeUInt8, "changed"};

#define ADD_SETTING(TYPE, NAME, DEFAULT) \
	col_name.column->insert(String(#NAME)); \
	col_value.column->insert(settings.NAME.toString()); \
	col_changed.column->insert(UInt64(settings.NAME.changed));

	APPLY_FOR_SETTINGS(ADD_SETTING)
#undef ADD_SETTING

#define ADD_LIMIT(TYPE, NAME, DEFAULT) \
	col_name.column->insert(String(#NAME)); \
	col_value.column->insert(settings.limits.NAME.toString()); \
	col_changed.column->insert(UInt64(settings.limits.NAME.changed));

	APPLY_FOR_LIMITS(ADD_LIMIT)
#undef ADD_LIMIT

	Block block{
		col_name,
		col_value,
		col_changed,
	};

	return BlockInputStreams(1, new OneBlockInputStream(block));
}


}
