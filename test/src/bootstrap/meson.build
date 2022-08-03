


####################################################################################################################################
# Test framework library. TODO: test framework is separate directory/subproject from the tests themselves.
####################################################################################################################################
libTest = archive_library( 'libTest',
    '../../src/build/common/render.c',
    '../../src/build/common/yaml.c',
    '../../src/command/command.c',
    '../../src/command/exit.c',
    '../../src/command/help/help.c',
	'../../src/common/compress/bz2/common.c',
	'../../src/common/compress/bz2/decompress.c',
    '../../src/common/ini.c',
    '../../src/common/io/fd.c',
    '../../src/common/io/fdRead.c',
    '../../src/common/io/fdWrite.c',
    '../../src/common/lock.c',
    '../../src/common/stat.c',
    '../../src/common/type/json.c',
    '../../src/config/config.c',
    '../../src/config/parse.c',
    )

executable( 'common_logTest'
    common/logTest.c,

    include_directories: [inc_backrest, inc_test],
    link_with: [lib_backrest, lib_test]
    dependencies: [lib_bz2,lib_yaml],
    build_by_default: false,
)
