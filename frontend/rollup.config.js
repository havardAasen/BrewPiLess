import resolve from '@rollup/plugin-node-resolve';
import commonjs from '@rollup/plugin-commonjs';
import terser from '@rollup/plugin-terser';

const isProduction = process.env.NODE_ENV === 'production';

export default {
  input: 'src/js/main.js',
  output: {
    file: 'dist/bundle.js',
    format: 'es',
    sourcemap: false
  },
  plugins: [
    resolve(),
    commonjs(),
    ...(isProduction ? [terser()] : [])
  ]
};
