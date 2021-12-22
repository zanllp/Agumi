include('./stdafx.as')

const path = path_calc(env().working_dir, env().process_arg.target)
const as = fs.read(path)
const tfv = generate_agumi_script_token(as, path)
const ast = parse_agumi_script(as, path)
fs.write(f('{}-ast.json', path), json.stringify(ast))
fs.write(f('{}-token.json', path), json.stringify(tfv))