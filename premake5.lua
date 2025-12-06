local PROJECT_NAME = "ImguiBase"

-- premake5.lua
workspace "MyWorkspace"
    configurations { "Debug", "Release" }
    platforms { "x64" }
    architecture "x86_64"
    

    project (PROJECT_NAME)
       kind "ConsoleApp"
       language "C++"
       cppdialect "c++20"
       targetdir "bin/%{cfg.buildcfg}"
       flags{"MultiProcessorCompile"}
    
       files {"src/**.cpp"}
       --files { }
       includedirs {"vcpkg_installed/x64-linux/include","src"}
       libdirs{"vcpkg_installed/x64-linux/lib"}
        links{"glfw3","GLEW","imgui","implot","glm"}
    
        filter "action:gmake2"
            links{"X11","GL"}
        filter "action:vs2022" 
            links{"opengl32"}
       filter "configurations:Debug"
          defines { "_DEBUG" }
          symbols "On"
       filter "configurations:Release"
          optimize "On"

function customClean()
    -- Specify the directories or files to be cleaned
    local dirsToRemove = {
--        "lib",
        "bin","obj",
        ".vs"
    }

    local filesToRemove = {
        "Makefile",
        "imgui.ini",
    }
    RemoveAllFilesThatMatch("make")
    RemoveAllFilesThatMatch("sln")
    RemoveAllFilesThatMatch("vcxproj")
    RemoveAllFilesThatMatch("filters")
    -- Remove specified directories
    for _, dir in ipairs(dirsToRemove) do
        if os.isdir(dir) then
            os.rmdir(dir)
            print("Removed directory: " .. dir)
        end
    end

    -- Remove specified files
    for _, file in ipairs(filesToRemove) do
        if os.isfile(file) then
            os.remove(file)
            print("Removed file: " .. file)
        end
    end
end
function RemoveAllFilesThatMatch(extension)

    local files = os.matchfiles(string.format("*.%s",extension))
    for _, file in ipairs(files) do
        print(string.format("Deleting %s",file))
        os.remove(file)
    end
end
-- Add the custom clean function to the clean action
newaction {
    trigger = "clean",
    description = "Custom clean action",
    execute = function()
        customClean()
    end
}
newaction
{
    trigger = "setup",
    description = "Setups up dependencies",
    execute = function()
        local isWindows = package.config:sub(1, 1) == '\\' -- check if path separator is '\'

        os.execute("git clone https://github.com/microsoft/vcpkg.git")

        if isWindows then
            os.execute("vcpkg\\bootstrap-vcpkg.bat")
            os.execute("vcpkg\\vcpkg install")
        else
            os.execute("./vcpkg/bootstrap-vcpkg.sh")
            --os.execute("sudo apt-get update && sudo apt-get install "..DevPackages.." -y")
            os.execute("./vcpkg/vcpkg install")
        end
    end

}