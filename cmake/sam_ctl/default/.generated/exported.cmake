set(DEPENDENT_MP_BIN2HEXsam_ctl_default_z0K41CAI "c:/Program Files/Microchip/xc32/v4.60/bin/xc32-bin2hex.exe")
set(DEPENDENT_DEPENDENT_TARGET_ELFsam_ctl_default_z0K41CAI ${CMAKE_CURRENT_LIST_DIR}/../../../../out/sam_ctl/default.elf)
set(DEPENDENT_TARGET_DIRsam_ctl_default_z0K41CAI ${CMAKE_CURRENT_LIST_DIR}/../../../../out/sam_ctl)
set(DEPENDENT_BYPRODUCTSsam_ctl_default_z0K41CAI ${DEPENDENT_TARGET_DIRsam_ctl_default_z0K41CAI}/${sourceFileNamesam_ctl_default_z0K41CAI}.c)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRsam_ctl_default_z0K41CAI}/${sourceFileNamesam_ctl_default_z0K41CAI}.c
    COMMAND ${DEPENDENT_MP_BIN2HEXsam_ctl_default_z0K41CAI} --image ${DEPENDENT_DEPENDENT_TARGET_ELFsam_ctl_default_z0K41CAI} --image-generated-c ${sourceFileNamesam_ctl_default_z0K41CAI}.c --image-generated-h ${sourceFileNamesam_ctl_default_z0K41CAI}.h --image-copy-mode ${modesam_ctl_default_z0K41CAI} --image-offset ${addresssam_ctl_default_z0K41CAI} 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRsam_ctl_default_z0K41CAI}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFsam_ctl_default_z0K41CAI})
add_custom_target(
    dependent_produced_source_artifactsam_ctl_default_z0K41CAI 
    DEPENDS ${DEPENDENT_TARGET_DIRsam_ctl_default_z0K41CAI}/${sourceFileNamesam_ctl_default_z0K41CAI}.c
    )
