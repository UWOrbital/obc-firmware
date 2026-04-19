def to_foreign_key_value(
    schema_name: str,
    foreign_table_name: str,
    foreign_column_name: str = "id",
) -> str:
    """
    Generates the foreign key column value to be used in foreign key constraints.
    This function is used if the tables are in the same schema.

    :usage: `foreign_key_column: type = Field(foreign_key=to_foreign_key_value(TABLE_NAME, id_field))`

    :note: This is the preferred way of creating foreign key constraints as it allows us to leverage SQLModel's type
        hinting. Unfortunately, SQLModel doesn't support creating foreign key constraints if the tables are in different
        schemas, thus `Column(type, ForeignKey(Table.id_field)) # type: ignore` is used

    :param foreign_table_name: The table to which to create a reference to.
    :param foreign_column_name: The column to reference to. The default is "id"

    :return: The foreign key value
    """
    foreign_key_value = f"{schema_name}.{foreign_table_name}.{foreign_column_name}"
    return foreign_key_value
