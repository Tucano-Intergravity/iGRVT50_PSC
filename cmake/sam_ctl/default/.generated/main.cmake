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


# Main target for this project
add_executable(sam_ctl_default_image_OOJykqNe ${sam_ctl_default_library_list})

set_target_properties(sam_ctl_default_image_OOJykqNe PROPERTIES
    OUTPUT_NAME "default"
    SUFFIX ".elf"
    RUNTIME_OUTPUT_DIRECTORY "${sam_ctl_default_output_dir}")
target_link_libraries(sam_ctl_default_image_OOJykqNe PRIVATE ${sam_ctl_default_default_XC32_FILE_TYPE_link})

# Add the link options from the rule file.
sam_ctl_default_link_rule( sam_ctl_default_image_OOJykqNe)

# Add bin2hex target for converting built file to a .hex file.
string(REGEX REPLACE [.]elf$ .hex sam_ctl_default_image_name_hex ${sam_ctl_default_image_name})
add_custom_target(sam_ctl_default_Bin2Hex ALL
    COMMAND ${MP_BIN2HEX} \"${sam_ctl_default_output_dir}/${sam_ctl_default_image_name}\"
    BYPRODUCTS ${sam_ctl_default_output_dir}/${sam_ctl_default_image_name_hex}
    COMMENT "Convert built file to .hex")
add_dependencies(sam_ctl_default_Bin2Hex sam_ctl_default_image_OOJykqNe)



