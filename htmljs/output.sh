#!/bin/bash

OUTDIR="cheader"

if [ ! -d $OUTDIR ]; then
    echo "$OUTDIR not found!"
    mkdir "$OUTDIR"
fi
rm -f $OUTDIR/*.h

htmlfiles=(index_s.htm.gz control_s.htm.gz config.htm.gz setup.htm.gz logging.htm.gz gravity.htm.gz pressure.htm.gz)
variables=(data_index_htm_gz control_htm_gz config_htm_gz setup_htm_gz logging_htm_gz gravity_htm_gz pressure_htm_gz)
outfiles=(index_htm control_htm config_htm setup_htm log_htm gdc_htm pressure_htm)
languages=(english spanish portuguese-br slovak chinese)

non_translated_files=(lcd.htm.gz edit.htm.gz testcmd.htm.gz bwf.gz.js dygraph.gz.js)
on_translated_variables=(lcd_htm_gz edit_htm_gz testcmd_htm_gz data_bwf_min_js_gz dygraph_combined_js_gz)
non_translated_out=(lcd_htm edit_html_gz testcmd_htm bwf_js dygraph_js)

gen_C_file()
{
lang=$1
for ((index=0; index<${#htmlfiles[@]}; index++)); do
    srcdir="dist/$lang"

#   echo "[$index]: ${htmlfiles[$index]}"
   input="$srcdir/${htmlfiles[$index]}"
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
   variable=${on_translated_variables[$index]}
   xxd -i "$input" > "$output"
   echo "processing $output"
   sed -i "s/unsigned char .\+\[\]/const unsigned char $variable\[\] PROGMEM/" "$output"
done