"""Audit the source-only export; build and Git metadata are deliberately excluded."""
from pathlib import Path
import hashlib, json, re, sys

root=Path(__file__).resolve().parents[1]
errors=[]
files=[]
for path in sorted(root.rglob('*')):
    rel=path.relative_to(root)
    if rel.parts[0] in ('.git','build') or '__pycache__' in rel.parts:
        continue
    if not path.is_file(): continue
    if path.is_symlink(): errors.append(f'Symlink: {rel}')
    if path.suffix.lower() in ('.rom','.col','.sg','.sms','.uf2','.elf','.bin','.o','.obj','.exe','.dll','.zip'):
        errors.append(f'Binary/content excluded from source release: {rel}')
    data=path.read_bytes()
    text=data.decode('utf-8',errors='replace')
    if path.resolve()!=Path(__file__).resolve() and (path.suffix.lower() in ('.c','.h','.cpp','.cmake','.ps1','.py','.sh') or path.name=='CMakeLists.txt'):
        if re.search(r'(?i)\b[A-Z]:[/\\]|/(?:Users|home)/[A-Za-z0-9._-]+/',text):
            errors.append(f'Private/local source reference: {rel}')
        if ('released as freeware for non-commercial' in text or
            'generate_z80_tables' in text or 'SN76489_Context* SN76489' in text):
            errors.append(f'Excluded implementation: {rel}')
    files.append({'path':rel.as_posix(),'sha256':hashlib.sha256(data).hexdigest()})
for name in ('LICENSE','LICENSE.BSD-3-Clause','THIRD_PARTY.md','dependencies.lock.json',
             'deps/chips/LICENSE','deps/emu76489/LICENSE','deps/smsplus/COPYING',
             'deps/pico_shared/LICENSE','deps/pico_shared/drivers/pico_fatfs/LICENSE'):
    if not (root/name).is_file():errors.append(f'Missing notice: {name}')
if errors:
    print('\n'.join(errors));sys.exit(1)
print(f'PASS source selection, required notices, no bundled content/private paths: {len(files)} files')
if len(sys.argv)>1:
    Path(sys.argv[1]).write_text(json.dumps(files,indent=2)+'\n',encoding='utf-8')
