# cmake files support debug production
include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(sam_ctl_default_library_list )

# Handle files with suffix s, for group default-XC32
if(sam_ctl_default_default_XC32_FILE_TYPE_assemble)
add_library(sam_ctl_default_default_XC32_assemble OBJECT ${sam_ctl_default_default_XC32_FILE_TYPE_assemble})
    sam_ctl_default_default_XC32_assemble_rule(sam_ctl_default_default_XC32_assemble)
    list(APPEND sam_ctl_default_library_list "$<TARGET_OBJECTS:sam_ctl_default_default_XC32_assemble>")

endif()

# Handle files with suffix S, for group default-XC32
if(sam_ctl_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
add_library(sam_ctl_default_default_XC32_assembleWithPreprocess OBJECT ${sam_ctl_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
    sam_ctl_default_default_XC32_assembleWithPreprocess_rule(sam_ctl_default_default_XC32_assembleWithPreprocess)
    list(APPEND sam_ctl_default_library_list "$<TARGET_OBJECTS:sam_ctl_default_default_XC32_assembleWithPreprocess>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(sam_ctl_default_default_XC32_FILE_TYPE_compile)
add_library(sam_ctl_default_default_XC32_compile OBJECT ${sam_ctl_default_default_XC32_FILE_TYPE_compile})
    sam_ctl_default_default_XC32_compile_rule(sam_ctl_default_default_XC32_compile)
    list(APPEND sam_ctl_default_library_list "$<TARGET_OBJECTS:sam_ctl_default_default_XC32_compile>")

endif()

# Handle files with suffix cpp, for group default-XC32
if(sam_ctl_default_default_XC32_FILE_TYPE_compile_cpp)
add_library(sam_ctl_default_default_XC32_compile_cpp OBJECT ${sam_ctl_default_default_XC32_FILE_TYPE_compile_cpp})
    sam_ctl_default_default_XC32_compile_cpp_rule(sam_ctl_default_default_XC32_compile_cpp)
    list(APPEND sam_ctl_default_library_list "$<TARGET_OBJECTS:sam_ctl_default_default_XC32_compile_cpp>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(sam_ctl_default_default_XC32_FILE_TYPE_dependentObject)
add_library(sam_ctl_default_default_XC32_dependentObject OBJECT ${sam_ctl_default_default_XC32_FILE_TYPE_dependentObject})
    sam_ctl_default_default_XC32_dependentObject_rule(sam_ctl_default_default_XC32_dependentObject)
    list(APPEND sam_ctl_default_library_list "$<TARGET_OBJECTS:sam_ctl_default_default_XC32_dependentObject>")

endif()

# Handle files with suffix elf, for group default-XC32
if(sam_ctl_default_default_XC32_FILE_TYPE_bin2hex)
add_library(sam_ctl_default_default_XC32_bin2hex OBJECT ${sam_ctl_default_default_XC32_FILE_TYPE_bin2hex})
    sam_ctl_default_default_XC32_bin2hex_rule(sam_ctl_default_default_XC32_bin2hex)
    list(APPEND sam_ctl_default_library_list "$<TARGET_OBJECTS:sam_ctl_default_default_XC32_bin2hex>")

endif()


# Main target for this project
add_executable(sam_ctl_default_image_z0K41CAI ${sam_ctl_default_library_list})

if(NOT CMAKE_HOST_WIN32)
    set_target_properties(sam_ctl_default_image_z0K41CAI PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${sam_ctl_default_output_dir})
endif()
set_target_properties(sam_ctl_default_image_z0K41CAI PROPERTIES OUTPUT_NAME "default")
set_target_properties(sam_ctl_default_image_z0K41CAI PROPERTIES SUFFIX ".elf")

target_link_libraries(sam_ctl_default_image_z0K41CAI PRIVATE ${sam_ctl_default_default_XC32_FILE_TYPE_link})


# Add the link options from the rule file.
sam_ctl_default_link_rule(sam_ctl_default_image_z0K41CAI)

# Call bin2hex function from the rule file
sam_ctl_default_bin2hex_rule(sam_ctl_default_image_z0K41CAI)

if(CMAKE_HOST_WIN32)
    add_custom_command(
        TARGET sam_ctl_default_image_z0K41CAI
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${sam_ctl_default_output_dir}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:sam_ctl_default_image_z0K41CAI> ${sam_ctl_default_output_dir}/${sam_ctl_default_original_image_name}
        BYPRODUCTS ${sam_ctl_default_output_dir}/${sam_ctl_default_original_image_name}
        COMMENT "Copying elf to out location")
    set_property(
        TARGET sam_ctl_default_image_z0K41CAI
        APPEND PROPERTY ADDITIONAL_CLEAN_FILES
        ${sam_ctl_default_output_dir}/${sam_ctl_default_original_image_name})
endif()

