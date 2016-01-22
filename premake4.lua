solution "DASH"
  configurations { "Debug", "Release" }

  targetdir "bin/"

  if _ACTION == "clean" then
    os.rmdir("bin/")
    os.rmdir("build/")
  end

  objdir "build/obj"
  location "build/"
  includedirs { "." }

--##################
--# Project: svcDASHmuxer
--##################

project "svcDASHmuxer"
  language "C++"
  kind "ConsoleApp"
  files { "src/**.cpp" }

configuration { "Debug*" }
    defines { "_DEBUG", "DEBUG" }
    flags   { "Symbols" }
    targetname ( "svcDASHmuxer_d" )

  configuration { "Release*" }
    defines { "NDEBUG" }
    flags   { "Optimize" }
    targetname ( "svcDASHmuxer" )

