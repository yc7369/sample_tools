#!/usr/bin/ python3

import os
import shutil
import platform



def main():
    release_dir = os.path.join(os.path.expanduser("~"), "release")
    
    home = os.path.expanduser("~")
    print(home)
    cwd = os.getcwd()
    print(cwd)

    if not os.path.exists(release_dir):
        os.mkdir(release_dir)
        os.chdir(release_dir)
        os.system("git clone --depth=1 git@172.24.13.60:sdsx/infra-release.git")

    release_dir = release_dir + "/infra-release"

    os.chdir(release_dir)
    os.system("git checkout .")
    os.system("git pull")

    os.chdir(cwd)

    print(release_dir)
    include_dir = release_dir + "/infrabase"
    if os.path.exists(include_dir):
        print("remove include:",include_dir)
        shutil.rmtree(release_dir + "/infrabase")
    lib_dir = release_dir + "/lib/"+ platform.system().lower()
    if os.path.exists(lib_dir):
        print("remove lib:",lib_dir)
        shutil.rmtree(lib_dir)
    # os.system("python3 ./convert_pub.py")

    copy_inc(release_dir,"./infrabase", "", None)

    copy_inc(release_dir,"./deps", "spdlog", None)
    copy_inc(release_dir,"./deps", "mjson", None)
    copy_inc(release_dir,"./deps", "encrypt", None)
    copy_inc(release_dir,"./deps", "libevent", "include")

    shutil.copytree("./deps/openssl/include/openssl",release_dir+"/infrabase/openssl")

    copy_inc(release_dir,"./src", "mybase", None)
    copy_inc(release_dir,"./src", "zlog", None)
    copy_inc(release_dir,"./src", "iomn", None)
    copy_inc(release_dir,"./src", "tcp", None)
    shutil.copytree("./src/gennerate_file", release_dir+"/infrabase/gennerate_file")
    print("copy sds_hare.h ",cwd,os.getcwd())
    copy_inc(release_dir,"./deps", "hare", "include")
    shutil.copy("./src/hare/sds_hare.h",release_dir+"/infrabase/hare/sds_hare.h")
    copy_lib(release_dir,".", platform.system().lower())
    shutil.copytree("./deps/hare/lib64/",release_dir+"/lib/hare/",dirs_exist_ok=True)

    #cmake 

    shutil.copy("./Release_CMake.txt",release_dir+"/CMakeLists.txt")
    shutil.copy("./Release_win.cmake",release_dir+"/Release_win.cmake")

    os.chdir(release_dir)
    push_git()

    


def copy_inc(release_dir,src, dst,inc):
    src_dir = src + "/" + dst + "/" + dst
    if (inc != None):
        src_dir = src + "/"+ dst + "/"  +inc
    dst_dir= release_dir + "/infrabase/" + dst
    print(src_dir,"=>",dst_dir)
    shutil.copytree(src_dir, dst_dir,dirs_exist_ok=True)


def copy_lib(release_dir,src, os_type):
    src_dir = src + "/lib/"
    dst_dir= release_dir + "/lib/"+ os_type+"/"
    print("copy lib",src_dir,"=>",dst_dir)
    shutil.copytree(src_dir, dst_dir,ignore=shutil.ignore_patterns('*.pdb', '*.exe',"*.exp"),dirs_exist_ok=True)

def push_git():
    os.system("git status")
    os.system("git add --all")
    os.system("git commit -m \"update\"")
    os.system("git push")
    os.system("git status")

if __name__ == "__main__":
    main()
