

const json_str = fs.read("/Users/wuqinchuan/Desktop/project/agumi/canada.json")

const j = json.parse(json_str)
@{
  const arr = []
  const fn = (node) => match (typeof(node)) {
      "object" : node.values().select(fn),
      "array": node.select(fn),
      _: arr.push(node)
    }
  fn(j)
  fs.write("./res.json", json.stringify(arr))
}
/*

const t = time()
for(5, () => {
  json.parse(json_str)
})
log((time() - t)/5)

*/
