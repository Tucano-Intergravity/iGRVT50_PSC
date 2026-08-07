set(DEPENDENT_MP_BIN2HEXsam_ctl_default_OOJykqNe "c:/Program Files/Microchip/xc32/v5.10/bin/xc32-bin2hex.exe")
set(DEPENDENT_DEPENDENT_TARGET_ELFsam_ctl_default_OOJykqNe "${CMAKE_CURRENT_LIST_DIR}/../../../../out/sam_ctl/default.elf")
set(DEPENDENT_TARGET_DIRsam_ctl_default_OOJykqNe "${CMAKE_CURRENT_LIST_DIR}/../../../../out/sam_ctl")
set(DEPENDENT_BYPRODUCTSsam_ctl_default_OOJykqNe ${DEPENDENT_TARGET_DIRsam_ctl_default_OOJykqNe}/${sourceFileNamesam_ctl_default_OOJykqNe}.c)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRsam_ctl_default_OOJykqNe}/${sourceFileNamesam_ctl_default_OOJykqNe}.c
    COMMAND ${DEPENDENT_MP_BIN2HEXsam_ctl_default_OOJykqNe} --image ${DEPENDENT_DEPENDENT_TARGET_ELFsam_ctl_default_OOJykqNe} --image-generated-c ${sourceFileNamesam_ctl_default_OOJykqNe}.c --image-generated-h ${sourceFileNamesam_ctl_default_OOJykqNe}.h --image-copy-mode ${modesam_ctl_default_OOJykqNe} --image-offset ${addresssam_ctl_default_OOJykqNe} 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRsam_ctl_default_OOJykqNe}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFsam_ctl_default_OOJykqNe})
add_custom_target(
    dependent_produced_source_artifactsam_ctl_default_OOJykqNe 
    DEPENDS ${DEPENDENT_TARGET_DIRsam_ctl_default_OOJykqNe}/${sourceFileNamesam_ctl_default_OOJykqNe}.c
    )
