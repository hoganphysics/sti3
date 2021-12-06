
#ifndef STI_PYTHON_STL_BIND_PYREF_H
#define STI_PYTHON_STL_BIND_PYREF_H


#include <pybind11/pybind11.h>
#include <pybind11/cast.h>
#include <pybind11/stl_bind.h>


#include <memory>

//Modified version from pybind11 library <pybind11/stl_bind.h> that keeps python object reference in a special 
//pyref_holder class. This holder class has a static global manager that holds the refernces
//Note that this is required because of a known limitation in pybind11 where the python part of a derived class
//is garbage collected even if a reference is kept in c++ of the base class. The manager class here store a
//pybind11::object reference of the python derived class while avoiding a circular reference.  
//See discussion here:
//https://github.com/pybind/pybind11/issues/1333


namespace STI
{
namespace Python
{

//**** Copied from pybind11/stl_bind.h with small modifications *****//

// Vector modifiers -- requires a copyable vector_type:
// (Technically, some of these (pop and __delitem__) don't actually require copyability, but it seems
// silly to allow deletion but not insertion, so include them here too.)
template <typename Vector, typename Class_, typename pyref_holder>
void vector_modifiers_pyref(pybind11::detail::enable_if_t<pybind11::detail::is_copy_constructible<typename Vector::value_type>::value, Class_> &cl) {
    using T = typename Vector::value_type;
    using SizeType = typename Vector::size_type;
    using DiffType = typename Vector::difference_type;

    auto wrap_i = [](DiffType i, SizeType n) {
        if (i < 0)
            i += n;
        if (i < 0 || (SizeType)i >= n)
            throw pybind11::index_error();
        return i;
    };

    cl.def("append",
           [](Vector &v, const T &value) {
                v.push_back(value);
                pyref_holder::addPyReference(value);
            },
           pybind11::arg("x"),
           "Add an item to the end of the list");

    cl.def(pybind11::init([](pybind11::iterable it) {
        auto v = std::unique_ptr<Vector>(new Vector());
        // pyref_holder::clearPyRefs();    //new vector; clear all old py reference in ref holder class
        v->reserve(pybind11::len_hint(it));
        for (pybind11::handle h : it)
           v->push_back(h.cast<T>());
           pyref_holder::addPyReference( v->back() );
        return v.release();
    }));

    cl.def("clear",
        [](Vector &v) {
            v.clear();
            pyref_holder::clearPyRefs();
        },
        "Clear the contents"
    );

    cl.def("extend",
       [](Vector &v, const Vector &src) {
           v.insert(v.end(), src.begin(), src.end());
           for(auto& o : src) {
               pyref_holder::addPyReference(o);
           }
       },
       pybind11::arg("L"),
       "Extend the list by appending all the items in the given list"
    );

    cl.def("extend",
       [](Vector &v, pybind11::iterable it) {
           const size_t old_size = v.size();
           v.reserve(old_size + len_hint(it));
           try {
               for (pybind11::handle h : it) {
                   v.push_back(h.cast<T>());
                   pyref_holder::addPyReference( v.back() );
               }
           } catch (const pybind11::cast_error &) {
               v.erase(v.begin() + static_cast<typename Vector::difference_type>(old_size), v.end());
               try {
                   v.shrink_to_fit();
               } catch (const std::exception &) {
                   // Do nothing
               }
               throw;
           }
       },
       pybind11::arg("L"),
       "Extend the list by appending all the items in the given list"
    );

    cl.def("insert",
        [](Vector &v, DiffType i, const T &x) {
            // Can't use wrap_i; i == v.size() is OK
            if (i < 0)
                i += v.size();
            if (i < 0 || (SizeType)i > v.size())
                throw pybind11::index_error();
            v.insert(v.begin() + i, x);
            pyref_holder::addPyReference(x);
        },
        pybind11::arg("i") , pybind11::arg("x"),
        "Insert an item at a given position."
    );

    cl.def("pop",
        [](Vector &v) {
            if (v.empty())
                throw pybind11::index_error();
            T t = v.back();
            v.pop_back();
            return t;
        },
        "Remove and return the last item"
    );

    cl.def("pop",
        [wrap_i](Vector &v, DiffType i) {
            i = wrap_i(i, v.size());
            T t = v[(SizeType) i];
            v.erase(v.begin() + i);
            return t;
        },
        pybind11::arg("i"),
        "Remove and return the item at index ``i``"
    );

    cl.def("__setitem__",
        [wrap_i](Vector &v, DiffType i, const T &t) {
            i = wrap_i(i, v.size());
            v[(SizeType)i] = t;
            pyref_holder::addPyReference(t);
        }
    );

    /// Slicing protocol
    cl.def("__getitem__",
        [](const Vector &v, pybind11::slice slice) -> Vector * {
            size_t start, stop, step, slicelength;

            if (!slice.compute(v.size(), &start, &stop, &step, &slicelength))
                throw pybind11::error_already_set();

            Vector *seq = new Vector();
            seq->reserve((size_t) slicelength);

            for (size_t i=0; i<slicelength; ++i) {
                seq->push_back(v[start]);
                start += step;
            }
            return seq;
        },
        pybind11::arg("s"),
        "Retrieve list elements using a slice object"
    );

    cl.def("__setitem__",
        [](Vector &v, pybind11::slice slice,  const Vector &value) {
            size_t start, stop, step, slicelength;
            if (!slice.compute(v.size(), &start, &stop, &step, &slicelength))
                throw pybind11::error_already_set();

            if (slicelength != value.size())
                throw std::runtime_error("Left and right hand size of slice assignment have different sizes!");

            for (size_t i=0; i<slicelength; ++i) {
                v[start] = value[i];
                start += step;
            }
        },
        "Assign list elements using a slice object"
    );

    cl.def("__delitem__",
        [wrap_i](Vector &v, DiffType i) {
            i = wrap_i(i, v.size());
            v.erase(v.begin() + i);
        },
        "Delete the list elements at index ``i``"
    );

    cl.def("__delitem__",
        [](Vector &v, pybind11::slice slice) {
            size_t start, stop, step, slicelength;

            if (!slice.compute(v.size(), &start, &stop, &step, &slicelength))
                throw pybind11::error_already_set();

            if (step == 1 && false) {
                v.erase(v.begin() + (DiffType) start, v.begin() + DiffType(start + slicelength));
            } else {
                for (size_t i = 0; i < slicelength; ++i) {
                    v.erase(v.begin() + DiffType(start));
                    start += step - 1;
                }
            }
        },
        "Delete list elements using a slice object"
    );

}


//**** Copied from pybind11/stl_bind.h with small modifications *****//

//
// std::vector
//
template <typename Vector, typename pyref_holder, typename holder_type = std::unique_ptr<Vector>, typename... Args>
pybind11::class_<Vector, holder_type> bind_vector_pyref(pybind11::handle scope, std::string const &name, Args&&... args) {
    using Class_ = pybind11::class_<Vector, holder_type>;

    // If the value_type is unregistered (e.g. a converting type) or is itself registered
    // module-local then make the vector binding module-local as well:
    using vtype = typename Vector::value_type;
    auto vtype_info = pybind11::detail::get_type_info(typeid(vtype));
    bool local = !vtype_info || vtype_info->module_local;

    Class_ cl(scope, name.c_str(), pybind11::module_local(local), std::forward<Args>(args)...);

    // Declare the buffer interface if a buffer_protocol() is passed in
    pybind11::detail::vector_buffer<Vector, Class_, Args...>(cl);

    cl.def(pybind11::init<>());

    // Register copy constructor (if possible)
    pybind11::detail::vector_if_copy_constructible<Vector, Class_>(cl);

    // Register comparison-related operators and functions (if possible)
    pybind11::detail::vector_if_equal_operator<Vector, Class_>(cl);

    // Register stream insertion operator (if possible)
    pybind11::detail::vector_if_insertion_operator<Vector, Class_>(cl, name);

    // Modifiers require copyable vector value type
    vector_modifiers_pyref<Vector, Class_, pyref_holder>(cl);

    // Accessor and iterator; return by value if copyable, otherwise we return by ref + keep-alive
    pybind11::detail::vector_accessor<Vector, Class_>(cl);

    cl.def("__bool__",
        [](const Vector &v) -> bool {
            return !v.empty();
        },
        "Check whether the list is nonempty"
    );

    cl.def("__len__", &Vector::size);




#if 0
    // C++ style functions deprecated, leaving it here as an example
    cl.def(init<size_type>());

    cl.def("resize",
         (void (Vector::*) (size_type count)) & Vector::resize,
         "changes the number of elements stored");

    cl.def("erase",
        [](Vector &v, SizeType i) {
        if (i >= v.size())
            throw index_error();
        v.erase(v.begin() + i);
    }, "erases element at index ``i``");

    cl.def("empty",         &Vector::empty,         "checks whether the container is empty");
    cl.def("size",          &Vector::size,          "returns the number of elements");
    cl.def("push_back", (void (Vector::*)(const T&)) &Vector::push_back, "adds an element to the end");
    cl.def("pop_back",                               &Vector::pop_back, "removes the last element");

    cl.def("max_size",      &Vector::max_size,      "returns the maximum possible number of elements");
    cl.def("reserve",       &Vector::reserve,       "reserves storage");
    cl.def("capacity",      &Vector::capacity,      "returns the number of elements that can be held in currently allocated storage");
    cl.def("shrink_to_fit", &Vector::shrink_to_fit, "reduces memory usage by freeing unused memory");

    cl.def("clear", &Vector::clear, "clears the contents");
    cl.def("swap",   &Vector::swap, "swaps the contents");

    cl.def("front", [](Vector &v) {
        if (v.size()) return v.front();
        else throw index_error();
    }, "access the first element");

    cl.def("back", [](Vector &v) {
        if (v.size()) return v.back();
        else throw index_error();
    }, "access the last element ");

#endif

    return cl;
}


} //Python
} //STI

#endif

