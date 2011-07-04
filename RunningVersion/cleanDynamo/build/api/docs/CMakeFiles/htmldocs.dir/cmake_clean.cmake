FILE(REMOVE_RECURSE
  "html"
  "latex"
  "rtf"
  "CMakeFiles/htmldocs"
  "Doxyfile"
  "footer.html"
  "genimages/interpose.eps"
  "genimages/windows.eps"
  "genimages/flow-highlevel.eps"
  "genimages/viper.eps"
  "genimages/client.eps"
  "genimages/flow.eps"
  "genimages/interpose.png"
  "genimages/windows.png"
  "genimages/flow-highlevel.png"
  "genimages/viper.png"
  "genimages/client.png"
  "genimages/flow.png"
  "html/index.html"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/htmldocs.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
