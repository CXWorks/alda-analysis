#!/usr/bin/python3

import fire
import json
from Cheetah.Template import Template


class opt_generator(object):
    def gen_opt_cpp(self, instr, tlp):
        binops = {'AddInst': 11, 'FAddInst': 12, 'SubInst':13, 'FSubInst': 14, 'MulInst': 15, 'FMul:Inst': 16,
                  'UDivInst':17, 'SDivInst': 18, 'FDivInst': 19, 'URemInst': 20, 'SRemInst': 21, 'FRemInst':22,
                  'ShlInst': 23, 'LShrInst': 24, 'AShrInst':25, 'AndInst':26, 'OrInst': 27, 'XorInst': 28}
        with open(instr, 'r') as f:
            instr_json = json.load(f)
        shift = int(instr_json['shift'])
        shift = (2 ** shift) - 1
        instr_json = instr_json['instr']
        meta = {}
        func_names = set()
        meta['functions'], llvm_inst, call_inst, global_inst = [], [], [], []
        has_return = False
        insert_loc = {}
        binop_general = None
        filer = {'LibcRead', 'LibcWrite'}
        # preprocess to detect condtions
        for node in instr_json:
            if(node['location'] in filer):
                continue
            if node['location'] not in insert_loc:
                insert_loc[node['location']] = []
            # if node['location'] == 'StoreInst' or node['location'] == 'LoadInst':
            #     has_size = False
            #     for p in node['params']:
            #         if 'sizeof' in p['name']:
            #             has_size = True
            #     if not has_size:
            #         if node['location'] == 'StoreInst':
            #             node['params'].append({'name': '$1 sizeof'})
            #             node['function']['params'].append({'name': 'size_', 'type': 'int64', 'cpp_type': 'int64', 'alias': 'p', 'sync': 'false', 'size': '0'})
            #         else:
            #             node['params'].append({'name': '$r sizeof'})
            #             node['function']['params'].append({'name': 'size_', 'type': 'int64', 'cpp_type': 'int64', 'alias': 'p', 'sync': 'false', 'size': '0'})


            insert_loc[node['location']].append(node)
        # detect if we have switch conditions
        # for k, v in insert_loc.items():
        #
        #     if (len(v) > 1):
        #         has = False
        #         for node in v:
        #             for p in node['params']:
        #                 if ('=' in p['name']):
        #                     has = True
        #                     break
        #         if (has):
        #             insert_loc[k] = [self.combine_switch_conditions(v)]
        extra_switch = 0
        for k, v in insert_loc.items():
            for node in v:
                unq = 0
                for i in range(len(node['params'])):

                    unit = node['params'][i]
                    unit['type'] = node['function']['params'][i]['type']
                    unq += 1
                    name = unit['name']
                    if('$t' in name):
                        continue
                    if ('.' in name):
                        unit['mapping'] = name[name.index('.') + 1:]
                        unit['idx'] = name[1:name.index('.')]
                        unit['unqi'] = unq
                    else:
                        if('sizeof' in name):
                            unit['sizeof'] = 1
                            node['function']['params'][i]['type'] = 'sizeof'
                            unit['type'] = 'sizeof'
                            unit['idx'] = name[1:name.find(' ')]
                        else:
                            unit['idx'] = name[1:]
                        unit['unqi'] = unq
                if node['location'].startswith('func '):
                    node['location'] = node['location'][len('func '):]
                    call_inst.append(node)
                elif node['location'].startswith('GLOBAL_'):
                    node['location'] = node['location'][len('GLOBAL_'):]
                    global_inst.append(node)
                else:
                    if(node['location'] in binops):
                        node['extra_loc_constraint'] = binops[node['location']]
                        node['location'] = 'BinaryOperator'
                        llvm_inst.append(node)
                    elif(node['location'] == 'BinaryOperator'):
                        binop_general = node
                        if 'return_info' in node:
                            has_return = True
                            meta['return_type'] = node['return_info']['type']
                    else:
                        llvm_inst.append(node)
                if node['function']['name'] not in func_names:
                    meta['functions'].append(node)
                    func_names.add(node['function']['name'])
                    if ('switch' in node):
                        extra_switch += len(node['switch_funcs'])
                if 'return_info' in node:
                    has_return = True
                    meta['return_type'] = node['return_info']['type']
        if(binop_general is not None):
            llvm_inst.append(binop_general)
        meta['callInst'] = call_inst
        meta['llvmInst'] = llvm_inst
        meta['globalInst'] = global_inst
        meta['total'] = len(meta['functions']) + extra_switch

        t = Template(file=tlp, searchList={'meta': meta, 'has_return': has_return, 'shift': shift})
        with open('../../pass/opt_gen.cpp', 'w') as f:
            f.write(str(t))

    def combine_switch_conditions(self, v):
        node_default = None
        cases = []
        for n in v:
            is_default = True
            for p in n['params']:
                if ('=' in p['name']):
                    is_default = False
                    break
            if (is_default):
                node_default = n
            else:
                cases.append(n)
        # combine_cases
        for i in range(len(node_default['params'])):

            for case in cases:
                if ('=' in case['params'][i]['name']):
                    if ('switch' not in node_default):
                        node_default['switch_funcs'] = {}
                        node_default['switch'] = True
                        node_default['switch_idx'] = i
                    node_default['switch_funcs'][int(case['params'][i]['name'][3:])] = case['function']['name']
        return node_default


if __name__ == '__main__':
    fire.Fire(opt_generator())
