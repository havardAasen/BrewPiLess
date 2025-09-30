import resolve from '@rollup/plugin-node-resolve';
import commonjs from '@rollup/plugin-commonjs';
import terser from '@rollup/plugin-terser';
import typescript from 'rollup-plugin-typescript2';


const isProduction = process.env.NODE_ENV === 'production';

export default {
  input: 'src/js/main.ts',
  output: {
    file: 'dist/bundle.js',
    format: 'es',
    sourcemap: false
  },
  plugins: [
    typescript(),
    resolve(),
    commonjs(),
    ...(isProduction ? [terser()] : [])
  ]
};
