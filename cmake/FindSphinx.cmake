if(ENABLE_DOC)
    find_program(SPHINX_EXECUTABLE
        NAMES sphinx-build
        DOC "Path to sphinx-build executable")

    include(FindPackageHandleStandardArgs)

    # Handle standard arguments to find_package like REQUIRED and QUIET
    find_package_handle_standard_args(Sphinx
        "Failed to find sphinx-build executable"
        SPHINX_EXECUTABLE)
endif()
