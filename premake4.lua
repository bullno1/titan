solution "Titan"
	configurations { "Develop", "Release" }

	configuration "vs2010"
		location "vs2010"
		flags {
			"Optimize",
			"OptimizeSpeed",
			"Symbols",
			"NoEditAndContinue"
		}

	configuration "Develop"
		targetdir "bin/dev"
		defines {
			"NDEBUG",
			"DEVELOP_BUILD"
		}

	configuration "Release"
		targetdir "bin/release"
		defines {
			"NDEBUG",
			"RELEASE_BUILD"
		}

	project "Titan"
		moaidir = os.getenv("MOAI_SDK")
		kind "ConsoleApp"
		language "C++"
		includedirs {
			"deps",
			moaidir .. "/3rdparty/LuaJIT-2.0.1/src",
			moaidir .. "/3rdparty/zlib-1.2.3",
			moaidir .. "/3rdparty/expat-2.0.1/lib",
			moaidir .. "/src",
			moaidir .. "/src/config-default"
		}
		files {
			"src/*.h",
			"src/*.cpp"
		}
		links {
			"FileWatcher",
			"SDL2"
		}

		configuration {"vs2010", "Release"}
			linkoptions {"/SUBSYSTEM:windows", "/ENTRY:mainCRTStartup"}

		LIB_DIR = moaidir .. "/cmake/projects/vs2010/"
		configuration "windows"
			defines {
				"WIN32",
				"_CONSOLE"
			}
			links {
				"kernel32",
				"user32",
				"gdi32",
				"winspool",
				"shell32",
				"ole32",
				"oleaut32",
				"uuid",
				"comdlg32",
				"advapi32",
				"winmm",
				"dxguid",
				"strmiids",
				"dsound",
				"psapi",
				"rpcrt4",
				"iphlpapi",
				"Ws2_32",
				"ws2_32",
				"glu32",
				"opengl32",
				LIB_DIR.."host-modules/Release/host-modules",
				LIB_DIR.."moai-sim/Release/moai-sim",
				LIB_DIR.."moai-util/Release/moai-util",
				LIB_DIR.."moai-core/Release/moai-core",
				LIB_DIR.."zlcore/Release/zlcore",
				LIB_DIR.."third-party/freeglut/Release/freeglutstatic",
				LIB_DIR.."moai-untz/Release/moai-untz",
				LIB_DIR.."third-party/untz-windows/Release/untz",
				LIB_DIR.."third-party/vorbis/Release/vorbis",
				LIB_DIR.."third-party/ogg/Release/ogg",
				LIB_DIR.."moai-box2d/Release/moai-box2d",
				LIB_DIR.."moai-chipmunk/Release/moai-chipmunk",
				LIB_DIR.."third-party/contrib/Release/contrib",
				LIB_DIR.."third-party/freetype/Release/freetype",
				LIB_DIR.."third-party/png/Release/png",
				LIB_DIR.."third-party/jpg/Release/jpg",
				LIB_DIR.."third-party/box2d/Release/box2d",
				LIB_DIR.."third-party/chipmunk/Release/chipmunk",
				LIB_DIR.."moai-http-client/Release/moai-http-client",
				LIB_DIR.."third-party/sfmt/Release/sfmt",
				LIB_DIR.."third-party/jansson/Release/jansson",
				LIB_DIR.."third-party/tinyxml/Release/tinyxml",
				LIB_DIR.."moai-luaext/Release/moai-luaext",
				LIB_DIR.."third-party/expat/Release/expat",
				LIB_DIR.."third-party/glew/Release/glew",
				LIB_DIR.."zlcore/zlvfs/Release/zlvfs",
				LIB_DIR.."third-party/tlsf/Release/tlsf",
				LIB_DIR.."third-party/luaext/luasocket/Release/luasocket",
				LIB_DIR.."third-party/luaext/luafilesystem/Release/luafilesystem",
				LIB_DIR.."third-party/luaext/luacurl/Release/luacurl",
				LIB_DIR.."third-party/curl/Release/curl",
				LIB_DIR.."third-party/zlib/Release/zlib",
				LIB_DIR.."third-party/ssl/Release/ssl",
				LIB_DIR.."third-party/luaext/luacrypto/Release/luacrypto",
				LIB_DIR.."third-party/crypto/Release/crypto",
				LIB_DIR.."third-party/luaext/luasql/Release/luasql",
				LIB_DIR.."third-party/luajit/luajit/src/lua51",
				LIB_DIR.."third-party/sqlite3/Release/sqlite3",
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
