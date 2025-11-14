#!/bin/bash

OUTDIR="cheader"

if [ ! -d $OUTDIR ]; then
    echo "$OUTDIR not found!"
    mkdir "$OUTDIR"
fi
rm -f $OUTDIR/*.h

translated_files=(index.htm.gz control.htm.gz config.htm.gz setup.htm.gz logging.htm.gz gravity.htm.gz pressure.htm.gz bundle.js)
variables=(data_index_htm_gz control_htm_gz config_htm_gz setup_htm_gz logging_htm_gz gravity_htm_gz pressure_htm_gz data_bundle_js)
outfiles=(index_htm control_htm config_htm setup_htm log_htm gdc_htm pressure_htm bundle_js)
languages=(english spanish portuguese-br slovak chinese)

non_translated_files=(lcd.htm.gz edit.htm.gz testcmd.htm.gz styles.min.css)
non_translated_variables=(lcd_htm_gz edit_htm_gz testcmd_htm_gz data_styles_min_css)
non_translated_out=(lcd_htm edit_html_gz testcmd_htm styles_min_css)

gen_C_file()
{
lang=$1
for ((index=0; index<${#translated_files[@]}; index++)); do
    srcdir="dist/$lang"

#   echo "[$index]: ${translated_files[$index]}"
   input="$srcdir/${translated_files[$index]}"
   output="$OUTDIR/${lang}_${outfiles[$index]}.h"
   variable=${variables[$index]}
   #echo "input: $input output file: $output with variables $variable "
   xxd -i "$input" > "$output"
   echo "processing $output"
   sed -i "s/unsigned char .\+\[\]/const unsigned char $variable\[\] PROGMEM/" "$output"
done
}

for lang in "${languages[@]}"; do
   gen_C_file "$lang"
done

for ((index=0; index<${#non_translated_files[@]}; index++)); do
   input="dist/${non_translated_files[$index]}"
   output="$OUTDIR/${non_translated_out[$index]}.h"
   variable=${non_translated_variables[$index]}
   xxd -i "$input" > "$output"
   echo "processing $output"
   sed -i "s/unsigned char .\+\[\]/const unsigned char $variable\[\] PROGMEM/" "$output"
done
