solution "CSL"
   configurations { "Debug", "Release" }

   project "CSLProgram"
      kind "ConsoleApp"
      language "C++"
      files { "../src/**.h", "../src/**.cpp" }
      --excludes { ../src/**.dummyfile }
      --libdirs { "../lib" }
      --links { "zlib" }
      --location "projectpath"
      targetdir "bin"
 
      configuration "Debug"
         defines { "DEBUG" }
         flags { "Symbols" }
         targetname "CSLProgram-Debug"
 
      configuration "Release"
         defines { "NDEBUG" }
         flags { "Optimize" }
         targetname "CSLProgram-Release"
