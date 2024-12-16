#pragma once

#define BOOST_CLASS_TEMPLATE_VERSION(Template, Type, Version)  \
	namespace boost                                            \
	{                                                          \
		namespace serialization                                \
		{                                                      \
			template <Template>                                \
			struct version<Type>                               \
			{                                                  \
				static constexpr unsigned int value = Version; \
			};                                                 \
			template <Template>                                \
			constexpr unsigned int version<Type>::value;       \
		}                                                      \
	}
