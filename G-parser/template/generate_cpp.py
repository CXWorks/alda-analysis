#!/usr/bin/python3

import fire
import json
import copy
from Cheetah.Template import Template

memory_opt = True

def rreplace(s, old, new, occurrence):
    li = s.rsplit(old, occurrence)
    return new.join(li)

class cxx_generator(object):
    def __init__(self):
        pthread_lock_info = {}
        pthread_lock_info['type'] = 'element'
        pthread_lock_info['type_name'] = 'pthread_mutex_t'
        pthread_lock_info['real_cpp_type'] = 'pthread_mutex_t'
        self.pthread_lock_info = pthread_lock_info
        self.type_dict = {}
        self.shift = 0
        self.otid = 0

    def to_compression(self, var):
        if(var['scalar'] in self.tid):
            return ''
        dec = 'std::map<{}, int> {}map;\nint {}_counter = 1;\npthread_spinlock_t {}_lock;\n'
        return dec.format(var['scalar'], var['scalar'], var['scalar'], var['scalar'])

    def get_cpp_type(self, unit, key=False):
        if 'real_cpp_type' in unit:
            self.type_dict[unit['scalar']] = unit['real_cpp_type']
        if unit['type'] == 'element':
            if (key and unit['real_cpp_type'] == 'void *'):
                return 'ull'
            return unit['real_cpp_type']
        elif unit['type'] == 'set':
            if ('size' in unit['element']):
                return '{}::bit_vector<{}>'.format('rt_lib', unit['element']['size'])
            return '{}::set<{}>'.format('rt_lib', self.get_cpp_type(unit['element']))
        elif unit['type'] == 'map':
            if ('size' in unit['key']):
                return '{}::array_map<{}, {}, {}>'.format('rt_lib', self.get_cpp_type(unit['key']),
                                                          self.get_cpp_type(unit['value']), unit['key']['size'])
            if not memory_opt:
                return '{}::map<{}, {}, {}, {}, {}>'.format('rt_lib', self.get_cpp_type(unit['key'], True),
                                                        self.get_cpp_type(unit['value']),
                                                        str(unit['scope'] == 'universe').lower(), self.sync, self.shift)
            else:
                return '{}::map<{}, {}, {}, {}, {}, {}>'.format('rt_lib', self.get_cpp_type(unit['key'], True),
                                                            self.get_cpp_type(unit['value']),
                                                            str(unit['scope'] == 'universe').lower(), self.sync,
                                                                self.shift, self.shift)
        elif unit['type'] == 'clazz':
            return unit['clazz_name']

    def to_declaration(self, unit):
        return '{} {};'.format(self.get_cpp_type(unit), unit['scalar'])

    def to_lock_pool_name(self, var):
        return '{}_locks'.format(var['scalar'])

    def is_collection(self, type):
        return 'Set' in type or 'Map' in type

    def get_func_params(self, unit):
        params = []

        for param in unit['params']:
            if not self.is_collection(param['type_name']):
                if 'insert' in param and param['insert'] == '$t':
                    pass
                elif (param['real_cpp_type'] == 'type'):
                    params.append('uint {}'.format(param['alias'] + '_ty'))
                elif ('size' not in param):
                    params.append('{} {}'.format(param['scalar'], param['alias']))
                else:
                    params.append('{} {}'.format(param['scalar'], param['alias'] + '_id'))
            elif self.is_collection(param['type_name']):
                params.append('{} {}'.format('void *', param['alias'] + '_id'))
        if (unit['var_arg'] == "true"):
            params.append('...')
        return ', '.join(params)

    def get_return(self, unit):
        if unit['return_value'] == 'void':
            return unit['return_value']
        elif self.is_collection(unit['return_type']):
            return 'void*'
        else:
            return unit['return_value']

    def to_func(self, unit, shift):
        special_func = None
        # if 'location' in unit and (unit['location'] == 'StoreInst' or unit['location'] == 'LoadInst'):
        #     has_size = False
        #     for p in unit['params']:
        #         if 'sizeof' in p['insert']:
        #             has_size = True
        #     if not has_size:
        #         new_unit = copy.deepcopy(unit)
        #         if unit['location'] == 'StoreInst':
        #             new_unit['params'].append({'type': 'element', 'type_name': 'BuiltinCPPType', 'real_cpp_type': 'int64', 'scalar': 'int64', 'alias': 'p', 'insert': '$1 sizeof'})
        #         else:
        #             new_unit['params'].append({'type': 'element', 'type_name': 'BuiltinCPPType', 'real_cpp_type': 'int64', 'scalar': 'int64', 'alias': 'p', 'insert': '$r sizeof'})
        #         header = '{} {}({}){{\n'.format(self.get_return(unit), unit['name'], self.get_func_params(new_unit))
        #         calls = ""
        #         addr = None
        #         for p in unit['params']:
        #             if p['insert'] != '$t':
        #                 calls+=p['alias']+","
        #             if p['real_cpp_type'] == "pointer":
        #                 addr = p['alias']
        #         body = """\n
        #          while (p > 0) {{
        #            {}_real({});
        #             {} = {} + {};
        #             p-={};
        #          }}
        #          }}\n
        #         """.format( unit['name'][:-1], calls[:-1], addr, addr, 2**shift, 2**shift)
        #         special_func = header + body
        #
        #         unit['name'] = unit['name'][:-1]+"_real"


        header = '{} {}({}){{\n'.format(self.get_return(unit), unit['name'], self.get_func_params(unit))
        body = []
        lock_list = []
        compression = """
            int {} = -1;
    if({}map.find({}) != {}map.end()){{
        {} = {}map[{}];
    }} else {{
        pthread_spin_lock(&{}_lock);
        if({}map.find({}) != {}map.end()){{
            {} = {}map[{}];
        }} else {{
            {} = {}_counter++;
            {}map[{}] = {};
        }}
        pthread_spin_unlock(&{}_lock);
    }}
        """
        most_once = False
        for param in unit['params']:
            if 'size' in param:
                if(param['scalar'] not in self.tid):
                    body.append(compression.format(
                        param['alias'], param['scalar'], param['alias'] + '_id', param['scalar'], param['alias'],
                        param['scalar'], param['alias'] + '_id',
                        param['scalar'], param['scalar'], param['alias'] + '_id', param['scalar'], param['alias'],
                        param['scalar'], param['alias'] + '_id',
                        param['alias'], param['scalar'], param['scalar'], param['alias'] + '_id', param['alias'],
                        param['scalar']
                    ))
                else:
                    if '$t' == param['insert']:
                        body.append('{} {} = get_id(pthread_self());'.format(param['scalar'], param['alias']))
                    else:
                        body.append('{} {} = get_id({}_id);'.format(param['scalar'], param['alias'], param['alias']))
            elif param['real_cpp_type'] == 'type':
                body.append('{} {}({}_ty);'.format(param['scalar'], param['alias'], param['alias']))
        if (unit['var_arg'] == 'true'):
            body.append('va_list ap;\n va_start(ap, {});\n'.format(unit['params'][-1]['alias']))
        # enter critical section
        body.append(unit['body'])
        stmt = ''
        for param in unit['params']:
            if ('sync' in param and param['sync'] == 'true'):
                stmt += 'ull {}_local_lock = get_addr({});pthread_spin_lock(&{}[{}_local_lock]);'.format(
                    param['alias'], param['alias'], self.to_lock_pool_name(param),
                    param['alias'])
                lock_list.append(param)
        if (',   = PTHREAD_LOCK' in body[-1]):
            body[-1] = body[-1].replace(',   = PTHREAD_LOCK', ';' + stmt, 1)
            body[-1] = body[-1].replace(',   = PTHREAD_LOCK', '')
            body[-1] = body[-1].replace('= PTHREAD_LOCK,', '')
        elif ('= PTHREAD_LOCK,' in body[-1]):
            body[-1] = body[-1].replace('= PTHREAD_LOCK,', ';' + stmt, 1)
            body[-1] = body[-1].replace(',   = PTHREAD_LOCK', '')
            body[-1] = body[-1].replace('= PTHREAD_LOCK,', '')
        else:
            body.insert(-2, stmt)

        if bool(unit['sync']):
            # body.append('pthread_mutex_unlock(&{});'.format(unit['name']+'Lock'))
            if (unit['return_value'] == 'void'):
                stmt = ''
                for param in reversed(lock_list):
                    stmt += 'pthread_spin_unlock(&{}[{}_local_lock]);'.format(self.to_lock_pool_name(param),
                                                                              param['alias'])
                if ('= PTHREAD_UNLOCK' in body[-1]):
                    body[-1] = rreplace(body[-1], '= PTHREAD_UNLOCK', stmt, 1)
                    body[-1] = body[-1].replace('= PTHREAD_UNLOCK', '')
                else:
                    body.append(stmt)
            else:
                temp = []
                for stmt in body:
                    if ('= PTHREAD_UNLOCK' in stmt):
                        temp.append(stmt.replace('= PTHREAD_UNLOCK', ''))
                    else:
                        temp.append(stmt)
                body = temp
                bs = body.pop()
                pos = bs.index('return')
                s = ''
                for param in reversed(lock_list):
                    s += 'pthread_spin_unlock(&{}[{}_local_lock]);\n'.format(self.to_lock_pool_name(param),
                                                                             param['alias'])
                body.append(bs[:pos] + s + bs[pos:])
        if (unit['var_arg'] == 'true'):
            body.append('va_end(ap);\n')
        tail = '\n}\n'
        if special_func is not None:
            tail += special_func
            header = "inline "+header
        return header + '\n'.join(body) + tail

    def to_clazz_def(self, clazz):
        template = 'class {} {{\npublic:\n{}}};\n'
        fields = ''
        for f in clazz['fields']:
            fields += self.to_declaration(f) + '\n'
        return template.format(str(clazz['clazz_name']), fields)

    def extract_clazz_def(self, map_def):
        ans = []
        if map_def['value']['type'] == 'clazz':
            for inner in map_def['value']['fields']:
                if inner['type'] == 'map':
                    ans.extend(self.extract_clazz_def(inner))
            ans.append(map_def['value'])
        return ans

    def to_tydef(self, unit):
        return 'typedef {} {};'.format(self.get_cpp_type(unit), unit['scalar'])

    def gen_cxx_cpp(self, instr, tlp, memory_optimize = True):
        global memory_opt
        memory_opt = memory_optimize
        self.tid = []
        with open(instr, 'r') as f:
            cxx_json = json.load(f)
        ds, functions, vars, clazz, tydef = [], [], [], [], []
        tid_size = 0
        self.shift = int(cxx_json['shift'])
        del cxx_json['shift']
        self.return_value = 'uint'
        self.sync = 'false'
        self.init = None
        self.libc = {}
        filters = {'LibcRead', 'LibcWrite', 'MemMoveInst', 'MemCpyInst'}
        for k, v in cxx_json.items():
            if v['type'] == 'func':
                functions.append(v)
                if (v['return_value'] != 'void'):
                    self.return_value = v['return_value']
                if('location' in v and v['location'] in filters):
                    self.libc[v['location']] = v['name']
                for p in v['params']:
                    if 'insert' in p and p['real_cpp_type'] == 'threadid':
                        self.tid.append(p['scalar'])
                        tid_size = p['size']
            elif v['type_def'] == 'false':
                ds.append(v)
                if v['type'] == 'map':
                    clazz.extend(self.extract_clazz_def(v))
                    if (v['scope'] == 'universe'):
                        self.init = v
            elif v['type'] == 'element':
                vars.append(v)
                if (v['real_cpp_type'] == 'threadid' and 'size' in v):
                    tid_size = int(v['size'])
                # if (v['type_name'] == 'BuiltinOThreadIdType'):
                #     self.otid = 1
                if ('sync' in v and v['sync'] == 'true'):
                    self.sync = 'true'
            if 'type_def' in v and v['type_def'] == 'true':
                tydef.append(v)
        for k, v in cxx_json.items():
            if v['type'] == 'func':
                for p in v['params']:
                    if 'insert' in p and p['insert'] != '$t' and p['real_cpp_type'] == "threadid":
                        self.otid = 1
        t = Template(file=tlp,
                     searchList={'ds': ds, 'function': functions, 'vars': vars, 'to_declaration': self.to_declaration,
                                 'tydef': tydef, 'to_tydef': self.to_tydef,
                                 'to_func': self.to_func, 'to_lock_pool_name': self.to_lock_pool_name, 'clazz': clazz,
                                 'to_clazz_def': self.to_clazz_def, 'to_compression': self.to_compression,
                                 'tid_size': int(tid_size), 'return_value': self.return_value, 'init': self.init,
                                 'cpp_type': self.get_cpp_type, 'sync': self.sync == 'true', 'shift': self.shift,
                                 'otid': self.otid, 'libc': self.libc, 'tid_scalar': self.tid})
        with open('../../rtLib/cxx_gen.cpp', 'w') as f:
            f.write(str(t))


if __name__ == '__main__':
    fire.Fire(cxx_generator())
