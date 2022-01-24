import os,sys,shutil,time
import argparse

def find_files(top, extensions=('.c', '.cpp', '.h'), exclude=('ult', 'googletest', 'classtrace', '.git')):
    res = []
    for root, dirs, files in os.walk(top):
        dirs[:] = [d for d in dirs if d not in exclude]
        #hard code to exclude mos_ file and _utils
        files = [os.path.join(root, file) for file in files if os.path.splitext(file)[1] in extensions and not file.startswith('mos_') and not file.startswith('mhw_') and not file.startswith('media_') and not file.endswith('_utils.h')]
        res.extend(files)
    return res

def chk_ADD_TRACE(file_path, class_info):
    with open(file_path, 'r', errors="ignore") as fh:
        #print(file_path)
        lines = fh.readlines()
        for line in lines:
            if line.find('MEDIA_CLASS_DEFINE_END') > -1:
                class_name = line.split('MEDIA_CLASS_DEFINE_END')[1][1:-2]
                if class_name not in class_info:
                    class_info[class_name] = [file_path]
                else:
                    class_info[class_name].append(file_path)

def genHeaderFile(top, offset_head_file):
    with open(offset_head_file, 'r') as fh:
        lines = fh.readlines()
        #the head file is not the origin/empty file, already has the offset info and do not need re-generate.
        if str(lines).count('#define OFFSET_') > 0:
            return

    files = find_files(top, ('.h'))
    files.sort()

    class_info = {}
    for file_name in files:
        chk_ADD_TRACE(file_name, class_info)

    class_list = list(class_info.keys())
    class_list.sort()

    with open(offset_head_file, 'w') as fh:
        fh.write('#ifndef __TRACE_OFFSET_H__\n')
        fh.write('#define __TRACE_OFFSET_H__\n')
        fh.write('enum {\n')
        for idx, class_name in enumerate(class_list):
            org_files = list(set(class_info[class_name]))
            org_files.sort()
            #for org_file in org_files:
            #    fh.write("  //%s\n"%org_file)
            fh.write("  OFFSET_%s,\n"%(class_name))
        fh.write('};\n')
        fh.write('#endif //__TRACE_OFFSET_H__\n')


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Command line inputs for checking script',argument_default=argparse.SUPPRESS)
    parser.add_argument('-w', '--workspace', help='workspace of driver, ', required=True)
    parser.add_argument('-f', '--headfile', help='the abs path of media_trace_offset.h', required=True)
    args = parser.parse_args()
    time_start = time.time()
    genHeaderFile(args.workspace, args.headfile)
    print("generate offset spend %0.4f s"%(time.time() - time_start))
