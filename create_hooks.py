import subprocess
import json
from os.path import exists


FORMAT_TABLE_START = 'struct ftrace_hook demo_hooks[] = {\n'
FORMAT_TABLE_END = '};'
FORMAT_TABLE_ROW = '\tHOOK(SYSCALL_NAME("sys_{name}"), fh_sys_{name}, &real_sys_{name}),\n'
FORMAT_FUNCTION_DECLARATION = 'asmlinkage long (*real_sys_{name})(struct pt_regs *regs);\n'

FORMAT_FUNCTION = 'asmlinkage long fh_sys_{name}(struct pt_regs *regs){{\n' \
	'\treturn fh_sys_generic(regs, real_sys_{name}, regs->{reg});\n' \
'}}\n'

GREP_CMD = 'grep -iEr "syscall_define[0-9]\((.*,){{{index}}} const char __user" {path_fs} | grep -oP "\(.*?," | grep -oE "(\w|\d\_)*"'
PATH_FS_SRC = '/root/kernels/linux-5.15/fs'
REGS = ['di', 'si', 'dx']
PATH_JSON = r'/root/projects/tagfs/tmp.json'

DIR_PATH = r'/root/projects/tagfs/hooks'





def save_json(syscalls):
    with open(PATH_JSON, 'w') as file:
        json.dump(syscalls, file)

def load_json():
    with open(PATH_JSON, 'r') as file:
        return json.loads(file.read())

def save_syscalls_list(syscalls, reg, output_grep):
    syscalls[reg] = [name.replace('\n', '') for name in output_grep.split('\n')]

def get_list_syscalls(syscalls):
    for index in range(len(REGS)):
        output = subprocess.getoutput(GREP_CMD.format(index=index + 1, path_fs=PATH_FS_SRC))
        save_syscalls_list(syscalls, REGS[index], output)

def get_function_names(reg):
    path = f'{DIR_PATH}/{reg}'

    with open(path, 'r') as file:
        return [name.replace('\n', '') for name in file.readlines()]

def add_hooks(syscalls, table, functions_declaration, functions, reg):
    functions_names = syscalls
    print(functions_names[0])
    for name in functions_names:
        table += FORMAT_TABLE_ROW.format(name=name)
        functions_declaration += FORMAT_FUNCTION_DECLARATION.format(name=name)
        functions += FORMAT_FUNCTION.format(name=name, reg=reg)
    return table, functions_declaration, functions

def create_hooks(syscalls):
    table = FORMAT_TABLE_START
    declaration = str()
    functions = str()

    for reg in REGS:
        table ,declaration, functions = add_hooks(syscalls[reg], table, declaration, functions, reg)

    table += FORMAT_TABLE_END

    return declaration + '\n' + functions + '\n' + table

def main():
    all_syscalls = dict()
    if(not exists(PATH_JSON)):
        get_list_syscalls(all_syscalls)
        save_json(all_syscalls)
    else:
        all_syscalls = load_json()

    data = create_hooks(all_syscalls)
    path = f'{DIR_PATH}/list_hooks.c'

    with open(path, 'w') as file:
        file.write(data)

if __name__ == '__main__':
    main()

# grep -iEr "syscall_define[0-9]\((.*,){1} const char __user" /root/kernels/linux-5.15/fs | grep -oP "\(.*?," | grep -oE "(\w|\d\_)*" > /root/projects/tagfs/hooks/di
# grep -iEr "syscall_define[0-9]\((.*,){2} const char __user" /root/kernels/linux-5.15/fs | grep -oP "\(.*?," | grep -oE "(\w|\d\_)*" > /root/projects/tagfs/hooks/si
# grep -iEr "syscall_define[0-9]\((.*,){3} const char __user" /root/kernels/linux-5.15/fs | grep -oP "\(.*?," | grep -oE "(\w|\d\_)*" > /root/projects/tagfs/hooks/dx