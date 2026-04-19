from alembic import op

"""
updating main table parameters to allow data size of 0

Revision ID: 6c54d182bb55
Revises: ee5986331915
Create Date: 2026-01-16 12:46:02.225339

"""


# revision identifiers, used by Alembic.
revision = "6c54d182bb55"
down_revision = "ee5986331915"
branch_labels = None
depends_on = None


def upgrade() -> None:
    """
    Allow the data size of main commands to be 0
    """
    op.create_check_constraint(
        "ck_main_commands_data_size_ge_0",  # OK
        "commands",
        "data_size >= 0",
        schema="main",
    )


def downgrade() -> None:
    """
    Revert the data size constraint on main commands
    """
    op.drop_constraint(
        "ck_main_commands_data_size_ge_0",
        "commands",
        type_="check",
        schema="main",
    )
