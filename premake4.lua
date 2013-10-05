solution "Titan"
	configurations { "Develop" }
	targetdir "bin"

	configuration "vs2010"
		location "vs2010"

	configuration "Develop"
		defines {
			"NDEBUG"
		}
		flags {
			"Optimize",
			"OptimizeSpeed",
			"Symbols",
			"NoEditAndContinue"
		}

	project "Titan"
		moaidir = os.getenv("MOAI_SDK")
		kind "ConsoleApp"
		language "C++"
		includedirs {
			"deps",
			moaidir .. "/3rdparty/lua-5.1.3/src",
			moaidir .. "/3rdparty/zlib-1.2.3",
			moaidir .. "/3rdparty/expat-2.0.1/lib",
			moaidir .. "/src",
			moaidir .. "/src/config-default",
			moaidir .. "/src/lua-headers"
		}
		files {
			"src/*.h",
			"src/*.cpp"
		}
		links {
			"FileWatcher",
			"SDL2"
		}

		configuration "windows"
			libdirs {
				moaidir .. "/vs2010/bin/Win32/Release-Lua-5.1"
			}
			defines {
				"WIN32",
				"_CONSOLE"
			}
			links {
				"moai-lib-box2d",
				"moai-lib-chipmunk",
				"moai-lib-core",
				"moai-lib-http-client",
				"moai-lib-http-server",
				"moai-lib-luaext",
				"moai-lib-sim",
				"moai-lib-untz",
				"moai-lib-util",

				"box2d",
				"chipmunk",
				"contrib",
				"expat",
				"freetype",
				"glew",
				"jansson",
				"libcares",
				"libcrypto",
				"libcurl",
				"libjpg",
				"libogg",
				"libpng",
				"libssl",
				"libvorbis",
				"lua-lib-5.1",
				"luaext",
				"mongoose",
				"sfmt",
				"sqlite",
				"tinyxml",
				"tlsf",
				"untz",
				"zlib",
				"zl-lib-gfx",
				"zl-lib-util",
				"zl-lib-vfs",

				"dsound",
				"strmiids",
				"opengl32",
				"advapi32",
				"comctl32",
				"oleaut32",
				"rpcrt4",
				"winmm",
				"wldap32",
				"ws2_32",
				"wsock32",
				"iphlpapi",
				"psapi"
			}

	project "FileWatcher"
		kind "StaticLib"
		language "C++"
		includedirs {
			"deps"
		}
		files {
			"deps/FileWatcher/*.h",
			"deps/FileWatcher/*.cpp"
		}
