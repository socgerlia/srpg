#pragma once

namespace RR{

template<class T, class U> struct copy_cvr{                        using type = U;                  };
template<class T, class U> struct copy_cvr<const T, U>{            using type = const U;            };
template<class T, class U> struct copy_cvr<volatile T, U>{         using type = volatile U;         };
template<class T, class U> struct copy_cvr<const volatile T, U>{   using type = const volatile U;   };
template<class T, class U> struct copy_cvr<T&, U>{                 using type = U&;                 };
template<class T, class U> struct copy_cvr<const T&, U>{           using type = const U&;           };
template<class T, class U> struct copy_cvr<volatile T&, U>{        using type = volatile U&;        };
template<class T, class U> struct copy_cvr<const volatile T&, U>{  using type = const volatile U&;  };
template<class T, class U> struct copy_cvr<T&&, U>{                using type = U&&;                };
template<class T, class U> struct copy_cvr<const T&&, U>{          using type = const U&&;          };
template<class T, class U> struct copy_cvr<volatile T&&, U>{       using type = volatile U&&;       };
template<class T, class U> struct copy_cvr<const volatile T&&, U>{ using type = const volatile U&&; };

}
