# quick and dirty model generation script...

require 'rexml/document'
include REXML

file = File.new("/home/dkrzemin/sipx/sipfoundry/main/sipXconfig/plugins/clearone/etc/clearone/C1MAXIP.txt")
doc = Document.new(file)
result = Document.new
model = result.add_element("model")
group = model.add_element("group")
group.attributes['name'] = 'basic'

comments = doc.root.comments
i = 0

for el in doc.root.elements
  setting = group.add_element("setting")
  setting.attributes["name"] = el.name
  value = setting.add_element('value')
  value.text = el.text[1..-2]
  description = setting.add_element('description')
  description.text = comments[i]
  i = i + 1
end

puts result


