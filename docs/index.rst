.. _indexsti:

.. STI Documentation master file, created by
   sphinx-quickstart on Mon Jan  4 18:25:08 2021.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.



.. toctree::
   :hidden:
   :maxdepth: 2

   src/readme.rst

.. toctree::
   :hidden:
   :caption: Features:
   :maxdepth: 3

   src/network
   src/device
   src/stipy
   src/webconsole

.. toctree::
   :hidden:
   :caption: Contents:
   :maxdepth: 2
   
   src/setuptools
   src/examples
   src/api
   src/techstack
   .. src/subtable



.. include :: src/readme.rst


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`



.. Docs
.. ====


..    digraph foo {
..       "bar" -> "baz";
..    }

.. ..
   .. doxygenindex::

.. .. inheritance-diagram:: sphinx.ext.inheritance_diagram.InheritanceDiagram
..    :parts: 1


.. .. inheritance-diagram:: STI::Engine::LocalEventEngine

.. .. doxygenclass:: STI::Engine::LocalEventEngine
..    :project: STI
..    :members: parse, play, trigger

.. .. doxygenclass:: STI::Engine::EventEngine
..    :project: STI
..    :members:
