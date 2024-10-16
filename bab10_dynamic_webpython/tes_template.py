from template_engine import \
    parse_template, \
    compile_template
fl = open('dokumen/template.html','r')
template_text = fl.read()
fl.close()
data_parameter = {
    'nama' : 'hantok panji saputro',
    'buah': ['Apel', 'Pisang', 'Nanas','alpukat']
} #Kompilasi template
tokens = parse_template(template_text)
output = compile_template(tokens, data_parameter)
print(output)