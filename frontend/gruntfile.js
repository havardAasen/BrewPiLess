'use strict';

module.exports = function(grunt) {
  grunt.loadNpmTasks('grunt-combo-html-css-js');
  grunt.loadNpmTasks('grunt-contrib-compress');
  grunt.loadNpmTasks('grunt-rollup');
  grunt.loadNpmTasks('grunt-contrib-copy');
  grunt.loadNpmTasks('grunt-contrib-htmlmin');
  grunt.loadNpmTasks('grunt-contrib-jshint');
  grunt.loadNpmTasks('grunt-contrib-sass');
  grunt.loadNpmTasks('grunt-contrib-uglify');
  grunt.loadNpmTasks('grunt-replace');
  grunt.loadNpmTasks('grunt-contrib-watch');
  grunt.loadNpmTasks('grunt-multi-lang-site-generator');
  grunt.loadNpmTasks('@lodder/grunt-postcss');
  grunt.loadNpmTasks('grunt-processhtml');

  const languages =  ['english', 'chinese', 'spanish', 'portuguese-br', 'slovak']
  const translations = {}

  languages.forEach(language => {
    translations[language] = grunt.file.readJSON(`src/locales/${language}.json`)
  })

  const replaceConfig = {}
  languages.forEach(language => {
    const patterns = Object.entries(translations[language])
    .map(([key, value]) => ({
      match: new RegExp(`<%=\\s*${key}\\s*%>`, 'g'),
      replacement: value,
    }))

    replaceConfig[language] = {
      options: {
        patterns,
        silent: true
      },
      files: [
        {
          expand: true,
          cwd: 'dist',
          src: ['bundle.js'],
          dest: `dist/${language}/`
        }
      ]
    }
  })

  grunt.initConfig({

    copy: {
      debug: {
        files: [
          {
            src: 'src/js/vendor/dygraph-combined.js',
            dest: 'dist/dygraph.js'
          }
        ]
      }
    },

    rollup: {
      default: {
        files: {
          'dist/bundle.js': ['src/js/main.js']
        }
      }
    },

    uglify: {
      target: {
        files: [
          { 'dist/bundle.js': 'dist/bundle.js' },
          { 'dist/dygraph.js': 'src/js/vendor/dygraph-combined.js' }
        ]
      }
    },

    htmlmin: {
      dist: {
        options: {
          removeComments: true,
          collapseWhitespace: true,
          collapseBooleanAttributes: true,
          removeAttributeQuotes: true,
          removeRedundantAttributes: false,
          removeEmptyAttributes: true,
          minifyJS: true,
          minifyCSS: true
        },
        files: {
          'dist/index.tmpl.htm': 'dist/index.tmpl.htm',
          'dist/control.tmpl.htm': 'dist/control.tmpl.htm',
          'dist/setup.tmpl.htm': 'dist/setup.tmpl.htm',
          'dist/gravity.tmpl.htm': 'dist/gravity.tmpl.htm',
          'dist/logging.tmpl.htm': 'dist/logging.tmpl.htm',
          'dist/config.tmpl.htm': 'dist/config.tmpl.htm',
          'dist/pressure.tmpl.htm': 'dist/pressure.tmpl.htm',
          'dist/BPLLogViewer.tmpl.htm': 'dist/BPLLogViewer.tmpl.htm',
          'dist/BPLog.tmpl.htm': 'dist/BPLog.tmpl.htm',
          'dist/lcd.htm': 'dist/lcd.htm',
          'dist/testcmd.htm': 'dist/testcmd.htm',
          'dist/edit.htm': 'dist/edit.htm'
        }
      }
    },

    comboall: {
      main: {
        files: [
          { 'dist/index.tmpl.htm': ['build/index.tmpl.html'] },
          { 'dist/control.tmpl.htm': ['build/control.tmpl.html'] },
          { 'dist/setup.tmpl.htm': ['build/setup.tmpl.html'] },
          { 'dist/gravity.tmpl.htm': ['build/gravity.tmpl.html'] },
          { 'dist/logging.tmpl.htm': ['build/logging.tmpl.html'] },
          { 'dist/config.tmpl.htm': ['build/config.tmpl.html'] },
          { 'dist/pressure.tmpl.htm': ['build/pressure.tmpl.html'] },
          { 'dist/BPLLogViewer.tmpl.htm': ['build/BPLLogViewer.tmpl.html'] },
          { 'dist/BPLog.tmpl.htm': ['build/BPLog.tmpl.html'] },
          { 'dist/lcd.htm': ['build/lcd.html']},
          { 'dist/testcmd.htm': ['src/testcmd.html']},
          { 'dist/edit.htm': ['src/edit.html']}
        ]
      }
    },

    jshint: {
      files: [
        'src/js/*.js'
      ],
      options: {
        curly: true,
        eqeqeq: true,
        immed: true,
        latedef: true,
        newcap: true,
        noarg: true,
        sub: true,
        undef: true,
        boss: true,
        eqnull: true,
        browser: true,
        globals: {
          console: true
        }
      }
    },

    sass: {
      dev: {
        options: {
          style: 'expanded'
        },
        files: [
          { expand: true, cwd: 'src/styles/', src: ['*.scss'], dest: './build/', ext: '.css' }
        ]
      }
    },

    postcss: {
      options: {
        map: true,
        processors: [ require('autoprefixer')() ]
      },
      dist: {
        src: 'build/*.css'
      }
    },

    compress: {
      main: {
        options: {
          mode: 'gzip',
          level: 9
        },
        files: [
          {
            expand: true,
            src: [
              'dist/*/*.htm',
              'dist/lcd.htm',
              'dist/testcmd.htm',
              'dist/edit.htm'
            ],
            dest: '.',
            ext: '.htm.gz'
          },
          {
            expand: true,
            src: ['dist/**/*.js'],
            dest: '.',
            ext: '.js'  // keep the same name for consistent HTML reference
          }
        ]
      }
    },

    processhtml: {
      dist: {
        files: {
          'build/index.tmpl.html': ['src/index.tmpl.html'],
          'build/control.tmpl.html': ['src/control.tmpl.html'],
          'build/setup.tmpl.html': ['src/setup.tmpl.html'],
          'build/gravity.tmpl.html': ['src/gravity.tmpl.html'],
          'build/logging.tmpl.html': ['src/logging.tmpl.html'],
          'build/config.tmpl.html': ['src/config.tmpl.html'],
          'build/pressure.tmpl.html': ['src/pressure.tmpl.html'],
          'build/BPLLogViewer.tmpl.html': ['src/BPLLogViewer.tmpl.html'],
          'build/BPLog.tmpl.html': ['src/BPLog.tmpl.html'],
          'build/lcd.html': ['src/lcd.html'],
          'build/testcmd.html': ['src/testcmd.html'],
          'build/edit.html': ['src/edit.html']
        }
      }
    },

    multi_lang_site_generator: {
      default: {
        options: {
          vocabs: languages,
          vocab_directory: 'src/locales',
          output_directory: 'dist',
          template_directory: 'dist'
        },
        files: [
          {'index.htm': ['index.tmpl.htm']},
          {'control.htm': ['control.tmpl.htm']},
          {'setup.htm': ['setup.tmpl.htm']},
          {'gravity.htm': ['gravity.tmpl.htm']},
          {'logging.htm': ['logging.tmpl.htm']},
          {'config.htm': ['config.tmpl.htm']},
          {'pressure.htm': ['pressure.tmpl.htm']},
          {'BPLog.htm': ['BPLog.tmpl.htm']},
          {'BPLLogViewer.htm': ['BPLLogViewer.tmpl.htm']}
        ]
      }
    },

    replace: replaceConfig,

    watch: {
      files: ['src/**/*'],
      tasks: ['default']
    }
  });

  const multi_lang_js_gen = languages.map(lang => `replace:${lang}`)

  grunt.registerTask('debug', [
    'copy',
    'rollup',
    'processhtml',
    'sass:dev',
    'postcss',
    'comboall',
    'multi_lang_site_generator',
      ...multi_lang_js_gen,
    'compress'
  ]);

  grunt.registerTask('default', [
    'copy',
    'rollup',
    'processhtml',
    'sass:dev',
    'postcss',
    'comboall',
    'watch'
  ]);

  grunt.registerTask('build', [
    'rollup',
    'uglify',
    'processhtml',
    'sass:dev',
    'postcss',
    'comboall',
    'htmlmin',
    'multi_lang_site_generator',
    ...multi_lang_js_gen,
    'compress'
  ]);
};
