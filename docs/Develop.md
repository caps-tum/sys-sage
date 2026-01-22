# Docs for Developers

## Naming conventions

- Use camel case.

- Variables start with a small letter while functions start with a capital
  letter.

- Functions with the "Get" prefix are considered to be true getters, i.e. they
  give direct (readonly) access to member variables or elements of member
  variables. This includes returning references or pointers.

- Functions with the "Find" prefix provide a collection of objects satisfying
  a certain condition. Their distinct feature is returning a newly created
  container holding the relevant objects. It is also possible to provide an
  already existing container to these functions.

- Functions with the "Calc" or "Count" prefix return information about the
  structure of the component tree.

- The following terms are used to describe components in the component tree
  relative to `this` component:

  - "parent": A component that is one level above and contains `this` component
              in the children vector.

  - "child": A component whose parent is `this` component.

  - "sibling": A component other than `this` component that is contained in the
               parent's children vector.

  - "ancestor": A component that is part of the shortest path starting from
                `this` component and going all the way up to the root of the
                component tree.

  - "descendant": A component that is part of the subtree spanned by `this`
                  component.
