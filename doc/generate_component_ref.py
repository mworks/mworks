from collections import defaultdict
from contextlib import closing, contextmanager
import glob
from io import StringIO
import json
import os
import string
import sys

import yaml


doc_dir = sys.argv[1]


def str_or_list(val):
    if isinstance(val, str):
        yield val
    else:
        assert isinstance(val, list)
        for item in val:
            yield item


basename_permitted_chars = string.ascii_lowercase + string.digits

def basename_for_name(name):
    name = ''.join((c if c in basename_permitted_chars else '_')
                   for c in name.lower())
    return '_'.join(part for part in name.split('_') if part)


all_basenames = set()
groups = {}
components = {}
group_members = defaultdict(set)
abstract_to_concrete = defaultdict(set)
abstract_to_group = {
    'Action': 'Actions',
    'Calibrator': 'Filters',
    'Filter': 'Filters',
    'IODevice': 'Input/Output',
    'Optimizer': 'Optimizers and Variable Monitors',
    'Resource': 'Resources',
    'Sound Stimulus': 'Sounds',
    'Stimulus': 'Stimuli',
    'Transition': 'Transitions',
}


def register_element(info):
    name = info['name']
    basename = basename_for_name(name)
    assert basename not in all_basenames, ('Duplicate basename: %r' %
                                           basename)
    info['basename'] = basename
    all_basenames.add(basename)

    if info.get('is_group', False):
        groups[name] = info
    else:
        alias = list(str_or_list(info.get('alias', [])))
        if alias:
            info['alias'] = alias
        mwel_alias = list(str_or_list(info.get('mwel_alias', [])))
        if mwel_alias:
            info['mwel_alias'] = mwel_alias
        group = set(str_or_list(info.get('group', [])))
        isa = set([name])
        allowed_child = set(str_or_list(info.get('allowed_child', [])))
        platform = list(str_or_list(info.get('platform', [])))
        parameters = info.get('parameters', [])
        param_names = set(p['name'] for p in parameters)
        for ancestor in str_or_list(info.get('isa', [])):
            ancestor = components[ancestor]
            group.update(ancestor['group'])
            isa.update(ancestor['isa'])
            if ('transient' not in info) and ('transient' in ancestor):
                info['transient'] = ancestor['transient']
            if ('toplevel' not in info) and ('toplevel' in ancestor):
                info['toplevel'] = ancestor['toplevel']
            allowed_child.update(ancestor['allowed_child'])
            platform = ancestor.get('platform', []) + platform
            for p in ancestor.get('parameters', []):
                if p['name'] not in param_names:
                    parameters.append(p)
                    param_names.add(p['name'])
        info['group'] = list(group)  # json.dump chokes on set
        info['isa'] = list(isa)
        info['allowed_child'] = list(allowed_child)
        if platform:
            info['platform'] = platform
        if parameters:
            info['parameters'] = parameters
        abstract = info.get('abstract', False)
        if not abstract:
            for ancestor_name in isa:
                if ancestor_name == name:
                    continue
                ancestor_info = components[ancestor_name]
                if ancestor_info.get('abstract', False):
                    abstract_to_concrete[ancestor_name].add(name)
        assert group or abstract, 'No groups for %r' % name
        for g in group:
            if not abstract:
                group_members[g].add(basename)
        components[name] = info


yaml_bundle_path = '/Resources/MWComponents.yaml'
yaml_files = ['/Library/Frameworks/MWorksCore.framework' + yaml_bundle_path]
yaml_files += glob.glob('/Library/Application Support/MWorks/Plugins/Core'
                        '/*.bundle/Contents' + yaml_bundle_path)

for filename in yaml_files:
    with open(filename) as fp:
        for info in yaml.safe_load_all(fp):
            register_element(info)

allowed_parents = defaultdict(set)
for name, info in components.items():
    for child_name in info['allowed_child']:
        allowed_parents[child_name].add(name)

# Serialize the components map for use by other tools
if not os.path.isdir(doc_dir):
    os.mkdir(doc_dir)
with open(os.path.join(doc_dir, 'components.json'), 'w') as fp:
    json.dump(components, fp)

os.chdir('components')


@contextmanager
def write_if_needed(filename):
    with closing(StringIO()) as fp:
        yield fp
        new_text = fp.getvalue()
    if os.path.isfile(filename):
        with open(filename) as fp:
            old_text = fp.read()
        if old_text == new_text:
            return
    with open(filename, 'w') as fp:
        fp.write(new_text)


warning_text = '''..
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   WARNING:  This is a generated file.  Do not edit it manually!
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
'''

def write_warning(fp):
    print(warning_text, file=fp)


def write_header(fp, title, underline_char='='):
    print(title, file=fp)
    print(underline_char * len(title), file=fp)


def write_index(basename, title, entries, description=''):
    with write_if_needed(basename + '.rst') as fp:
        write_warning(fp)
        print('.. _%s:\n' % title, file=fp)
        write_header(fp, title)
        if description:
            print('\n' + description, file=fp)
        print('\n.. toctree::\n   :maxdepth: 1\n', file=fp)
        for name in sorted(entries):
            print('   ' + name, file=fp)


def process_group(info):
    name = info['name']
    write_index(info['basename'],
                name,
                group_members.pop(name),
                info.get('description', ''))


write_index('index',
            'Components',
            [info['basename'] for info in groups.values()],
            '''\
Components are the building blocks of MWorks experiments.  Variables,
input/output devices, visual stimuli, sounds, control-flow constructs, and more
are all components.  To create an experiment, one must select, configure, and
connect the required components.
''')

for info in groups.values():
    process_group(info)

assert not group_members, ('Unhandled groups: %s' %
                           ', '.join(sorted(group_members.keys())))


def write_parameters(fp, kind, params):
    print('\n', file=fp)
    write_header(fp, kind + ' Parameters', '-')
    print(file=fp)

    for p in params:
        write_header(fp, p['name'], '^')

        if 'alias' in p:
            alias = p['alias']
            if not isinstance(alias, list):
                print(':Alias: %s' % alias, file=fp)
            else:
                print(':Aliases: | %s' % alias[0], file=fp)
                for a in alias[1:]:
                    print('          | %s' % a, file=fp)

        if 'recommended' in p:
            print(':Recommended: ``%s``' % p['recommended'], file=fp)

        if 'options' in p:
            options = p['options']
            print(':Options: | ``%s``' % options[0], file=fp)
            for o in options[1:]:
                print('          | ``%s``' % o, file=fp)

        if 'default' in p:
            print(':Default: ``%s``' % p['default'], file=fp)

        if 'example' in p:
            example = p['example']
            if not isinstance(example, list):
                print(':Example: ``%s``' % example, file=fp)
            else:
                print(':Examples: | ``%s``' % example[0], file=fp)
                for e in example[1:]:
                    print('           | ``%s``' % e, file=fp)

        if 'description' in p:
            print('\n%s' % p['description'], file=fp)

        print(file=fp)


def add_relation(name, relations):
    info = components[name]
    if not info.get('abstract', False):
        relations.add(name)
    elif name not in abstract_to_group:
        relations.update(abstract_to_concrete[name])
    else:
        relations.add('%s <%s>' % (name, abstract_to_group[name]))


def write_entry(title, info):
    with write_if_needed(info['basename'] + '.rst') as fp:
        write_warning(fp)
        print('.. index::', file=fp)
        print('   single: ' + title, file=fp)
        for group in sorted(info['group']):
            print('   single: %s; %s' % (group, title), file=fp)
        print(file=fp)

        print('.. _%s:\n' % title, file=fp)

        write_header(fp, title)

        if 'platform' in info:
            platforms = [{'macos': 'macOS', 'ios': 'iOS'}[p.lower()]
                         for p in info['platform']]
            availability = '\n*Available on '
            all_but_last = platforms[:-1]
            if all_but_last:
                if len(all_but_last) > 1:
                    availability += ', '.join(all_but_last) + ', and '
                else:
                    availability += all_but_last[0] + ' and '
            availability += platforms[-1] + '*'
            print(availability, file=fp)
        
        if 'description' in info:
            print('\n', file=fp)
            write_header(fp, 'Description', '-')
            print(info['description'], file=fp)
        
        if 'signature' in info:
            print('\n', file=fp)
            write_header(fp, 'Signature', '-')
            print('``%s``' % info['signature'], file=fp)
            if ('alias' in info) or ('mwel_alias' in info):
                print(file=fp)
                aliases = [('``%s``' % a) for a in info.get('alias', [])]
                aliases.extend([('``%s`` *(MWEL only)*' % a)
                                for a in info.get('mwel_alias', [])])
                aliases.sort()
                for a in aliases:
                    print(':Alias:', a, file=fp)
        
        if 'parameters' in info:
            required = []
            optional = []
            deprecated = []
            for p in info['parameters']:
                if p.get('required', False):
                    required.append(p)
                elif p.get('deprecated', False):
                    deprecated.append(p)
                else:
                    optional.append(p)
            if required:
                write_parameters(fp, 'Required', required)
            if optional:
                write_parameters(fp, 'Optional', optional)
            if deprecated:
                write_parameters(fp, 'Deprecated', deprecated)

        print('\n', file=fp)
        write_header(fp, 'Placement', '-')
        print('''.. list-table::
   :widths: auto
   :stub-columns: 1
''', file=fp)
        print('   * - Allowed at top level:', file=fp)
        print('     -',
              ('Yes' if info.get('toplevel', False) else 'No'),
              file=fp)
        allowed_parent = set()
        for ancestor_name in info['isa']:
            for parent_name in allowed_parents[ancestor_name]:
                add_relation(parent_name, allowed_parent)
        if allowed_parent:
            print('   * - Allowed parent:', file=fp)
            print('     -',
                  ', '.join(('`%s`' % c) for c in sorted(allowed_parent)),
                  file=fp)
        allowed_children = set()
        for child_name in info['allowed_child']:
            add_relation(child_name, allowed_children)
        if allowed_children:
            print('   * - Allowed children:', file=fp)
            print('     -',
                  ', '.join(('`%s`' % c) for c in sorted(allowed_children)),
                  file=fp)


for name, info in components.items():
    if not info.get('abstract', False):
        write_entry(name, info)
