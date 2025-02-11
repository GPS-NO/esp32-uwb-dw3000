Import('env')
import os
import shutil

OUTPUT_DIR = "build_output{}".format(os.path.sep)

def hex_map_copy(source, target, env):
    variant = str(target[0]).split(os.path.sep)[2]
    
    # check if output directories exist and create if necessary
    if not os.path.isdir(OUTPUT_DIR):
        os.mkdir(OUTPUT_DIR)

    for d in ['firmware', 'map']:
        if not os.path.isdir("{}{}".format(OUTPUT_DIR, d)):
            os.mkdir("{}{}".format(OUTPUT_DIR, d))

    # create string with location and file names based on variant
    map_file = "{}map{}{}.map".format(OUTPUT_DIR, os.path.sep, variant)
    hex_file = "{}firmware{}{}.hex".format(OUTPUT_DIR, os.path.sep, variant)

    # check if new target files exist and remove if necessary
    for f in [map_file, hex_file]:
        if os.path.isfile(f):
            os.remove(f)

    # copy firmware.hex to firmware/<variant>.hex
    shutil.copy(str(target[0]), hex_file)

    # copy firmware.map to map/<variant>.map
    if os.path.isfile("firmware.map"):
        shutil.move("firmware.map", map_file)

def elf_copy(source, target, env):
    variant = str(target[0]).split(os.path.sep)[2]
    
    # check if output directories exist and create if necessary
    if not os.path.isdir(OUTPUT_DIR):
        os.mkdir(OUTPUT_DIR)

    for d in ['firmware']:
        if not os.path.isdir("{}{}".format(OUTPUT_DIR, d)):
            os.mkdir("{}{}".format(OUTPUT_DIR, d))

    # create string with location and file names based on variant
    elf_file = "{}firmware{}{}.elf".format(OUTPUT_DIR, os.path.sep, variant)

    # check if new target files exist and remove if necessary
    for f in [elf_file]:
        if os.path.isfile(f):
            os.remove(f)

    # copy firmware.hex to firmware/<variant>.hex
    shutil.copy(str(target[0]), elf_file)

def bin_copy(source, target, env):
    variant = str(target[0]).split(os.path.sep)[2]
    
    # check if output directories exist and create if necessary
    if not os.path.isdir(OUTPUT_DIR):
        os.mkdir(OUTPUT_DIR)

    for d in ['firmware']:
        if not os.path.isdir("{}{}".format(OUTPUT_DIR, d)):
            os.mkdir("{}{}".format(OUTPUT_DIR, d))

    # create string with location and file names based on variant
    bin_file = "{}firmware{}{}.bin".format(OUTPUT_DIR, os.path.sep, variant)

    # check if new target files exist and remove if necessary
    for f in [bin_file]:
        if os.path.isfile(f):
            os.remove(f)

    # copy firmware.hex to firmware/<variant>.hex
    shutil.copy(str(target[0]), bin_file)

env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", [hex_map_copy])
env.AddPostAction("$BUILD_DIR/${PROGNAME}.elf", [elf_copy])
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", [bin_copy])