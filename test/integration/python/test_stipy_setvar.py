"""STIPy setvar behavior."""

import pytest


def test_makeshot_duplicate_setvar_raises_value_error(stipy_modules):
    stipy, _ = stipy_modules

    def shotmaker():
        stipy.setvar("duplicate_var", 1)
        stipy.setvar("duplicate_var", 2)

    with pytest.raises(ValueError, match="Variable 'duplicate_var' already defined"):
        stipy.makeshot(shotmaker)


def test_makeshot_vars_override_allows_first_setvar_declaration(stipy_modules):
    stipy, _ = stipy_modules

    def shotmaker():
        stipy.setvar("overridden_var", 1)

    shot = stipy.makeshot(shotmaker, vars={"overridden_var": 2})

    assert shot.rootgroup().var("overridden_var") == 2
