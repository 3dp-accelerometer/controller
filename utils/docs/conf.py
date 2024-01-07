#!/usr/bin/env python3

# stdlib
import os
import re
import sys

# 3rd party
from sphinx_pyproject import SphinxConfig

sys.path.append(".")

config = SphinxConfig(style="poetry", globalns=globals())
project = config["project"]
author = config["author"]
documentation_summary = config.description

github_url = "https://github.com/{github_username}/{github_repository}".format_map(config)

rst_prolog = f""".. |pkgname| replace:: sphinx-pyproject
.. |pkgname2| replace:: ``sphinx-pyproject``
.. |browse_github| replace:: `Browse the GitHub Repository <{github_url}>`__
"""
rst_epilog = """
"""

slug = re.sub(r"\W+", "-", project.lower())
release = version = config.version

sphinx_builder = os.environ.get("SPHINX_BUILDER", "html").lower()
todo_include_todos = int(os.environ.get("SHOW_TODOS", 0)) and sphinx_builder != "latex"

intersphinx_mapping = {
    "python": ("https://docs.python.org/3/", None),
    "sphinx": ("https://www.sphinx-doc.org/en/stable/", None),
    "sphinx-toolbox": ("https://sphinx-toolbox.readthedocs.io/en/latest", None),
}

html_show_sphinx = False
html_theme_options = {
    "logo_only": False,
    "prev_next_buttons_location": None
}
html_show_sourcelink = False
html_context = {
    "display_github": False,
    "github_user": config["github_username"],
    "github_repo": config["github_repository"],
    "github_version": "master",
    "conf_py_path": "/docs/",
}

htmlhelp_basename = slug

latex_documents = [("index", f"{slug}.tex", project, author, "manual")]
man_pages = [("index", slug, project, [author], 1)]
texinfo_documents = [("index", slug, project, author, slug, project, "Miscellaneous")]

# toctree_plus_types = set(config["toctree_plus_types"])

#autodoc_default_options = {
#    "members": None,  # Include all members (methods).
#    "special-members": None,
#    "autosummary": False,
#    "show-inheritance": None,
#    "exclude-members": ",".join(config["autodoc_exclude_members"]),
#    "autodoc_preserve_defaults": True,
#    "autodoc_warningiserror": True,
#    "typehints_fully_qualified": False,
#    "typehints_use_signature": True,
#    "typehints_use_signature_return": True,
#}

modindex_common_prefix = ["3dpaxxel."]

latex_elements = {
    "printindex": "\\begin{flushleft}\n\\printindex\n\\end{flushleft}",
    "tableofcontents": "\\pdfbookmark[0]{\\contentsname}{toc}\\sphinxtableofcontents",
}


def setup(app):
    # 3rd party
    from sphinx_toolbox.latex import better_header_layout

    app.connect("config-inited", lambda app, config: better_header_layout(config))


needspace_amount = r"5\baselineskip"

breathe_projects = {
    "My Proj": "./_gen_doxygen_docs/xml"
}

breathe_default_project = "My Proj"

exhale_args = {
    # These arguments are required
    "containmentFolder":     "./api",
    "rootFileName":          "library_root.rst",
    "rootFileTitle":         "Library API",
    "doxygenStripFromPath":  "..",
    "createTreeView":        True,
}

primary_domain = 'c'
highlight_language = 'c'
