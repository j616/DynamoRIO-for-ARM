FILE(REMOVE_RECURSE
  "ldscript"
  "CMakeFiles/drpreload.dir/linux/preload.c.o"
  "CMakeFiles/drpreload.dir/config.c.o"
  "CMakeFiles/drpreload.dir/linux/os.c.o"
  "CMakeFiles/drpreload.dir/Arm/arm.s.o"
  "../event_strings.h"
  "../lib/libdrpreload.pdb"
  "../lib/libdrpreload.so"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang ASM C)
  INCLUDE(CMakeFiles/drpreload.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
