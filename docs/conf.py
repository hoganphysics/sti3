# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import sys
sys.path.insert(0, os.path.abspath('.'))


# -- Project information -----------------------------------------------------

project = 'STI'
copyright = '2022, Jason Hogan'
author = 'Jason Hogan'

# The full version, including alpha/beta/rc tags
release = '3.0.0'


# -- General configuration ---------------------------------------------------

#sys.path.append( "/home/hogan/.local/lib/python3.6/site-packages/breathe" )

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = ["sphinx.ext.graphviz", "sphinx.ext.inheritance_diagram", "breathe", 'sphinx.ext.autosectionlabel', 'sphinx_tabs.tabs' ]

breathe_projects = {
"STI": "doxygen/xml/",
}
# Breathe Configuration
breathe_default_project = "STI"

breathe_domain_by_extension = {
        "h" : "cpp",
        }

breathe_default_members = ('members', 'undoc-members')

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = [
        '_build',
        'Thumbs.db',
        '.DS_Store',
        'src/archive',
        'src/setuptools_legacy.rst',
        ]


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'

html_theme_options = {
        'collapse_navigation': False,
        'navigation_depth': 4,
        'titles_only': False,
        }

html_use_index = False
html_domain_indices = False

numfig = True

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
#html_static_path = ['_static']
html_static_path = []

# Tabs
sphinx_tabs_valid_builders = ['linkcheck']
sphinx_tabs_disable_tab_closing = True
