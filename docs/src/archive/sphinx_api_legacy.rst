
=======================
Legacy Sphinx API Test
=======================

This page was an early experiment for embedding Doxygen output directly into
Sphinx with Breathe.  It is archived because the maintained API reference is
the standalone Doxygen output linked from ``api.rst``.


.. graphviz::

   digraph foo {
      "bar" -> "baz";
   }

..
   doxygenindex::

.. inheritance-diagram:: sphinx.ext.inheritance_diagram.InheritanceDiagram
   :parts: 1


.. inheritance-diagram:: STI::Engine::LocalEventEngine

.. doxygenclass:: STI::Engine::LocalEventEngine
   :project: STI
   :members: parse, play, trigger

.. doxygenclass:: STI::Engine::EventEngine
   :project: STI
   :members:
