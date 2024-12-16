import argparse
import os
import zipfile
import time

def make_rel_archive(a_args):
    archive = zipfile.ZipFile(a_args.name + ".zip", "w", zipfile.ZIP_DEFLATED)

    def do_write(a_path):
        os.utime(a_path, (time.time(), time.time()))
        archive.write(a_path, "F4SE/Plugins/{}".format(os.path.basename(a_path)))

    def write_fontfile(a_path):
        os.utime(a_path, (time.time(), time.time()))
        archive.write(a_path, "F4SE/Plugins/HFonts/{}".format(os.path.basename(a_path)))

    def write_rootfile(a_extension):
        do_write("{}/{}{}".format(a_args.src_dir, a_args.name, a_extension))

    do_write(a_args.dll)
    write_rootfile(".ini")

    font_file_path = os.path.join(a_args.src_dir, "droidsans.font")
    write_fontfile(font_file_path)

def parse_arguments():
    parser = argparse.ArgumentParser(description="archive build artifacts for distribution")
    parser.add_argument("--dll", type=str, help="the full dll path", required=True)
    parser.add_argument("--name", type=str, help="the project name", required=True)
    parser.add_argument("--out-dir", type=str, help="the output directory", required=True)
    parser.add_argument("--src-dir", type=str, help="the project root source directory", required=True)
    return parser.parse_args()

def main():
    args = parse_arguments()

    os.makedirs(args.out_dir, exist_ok=True)
    os.chdir(args.out_dir)

    make_rel_archive(args)

if __name__ == "__main__":
    main()
