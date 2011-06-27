FILE(REMOVE_RECURSE
  "CMakeFiles/api_headers"
  "include/dr_app.h"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/api_headers.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
