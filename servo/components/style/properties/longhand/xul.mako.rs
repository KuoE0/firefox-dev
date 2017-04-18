/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

<%namespace name="helpers" file="/helpers.mako.rs" />
<% from data import Method %>

// Non-standard properties that Gecko uses for XUL elements.
<% data.new_style_struct("XUL", inherited=False) %>

${helpers.single_keyword("-moz-box-align", "stretch start center baseline end",
                         products="gecko", gecko_ffi_name="mBoxAlign",
                         gecko_enum_prefix="StyleBoxAlign",
                         gecko_inexhaustive=True,
                         animation_type="none",
                         alias="-webkit-box-align",
                         spec="Nonstandard (https://developer.mozilla.org/en-US/docs/Web/CSS/box-align)")}

${helpers.single_keyword("-moz-box-direction", "normal reverse",
                         products="gecko", gecko_ffi_name="mBoxDirection",
                         gecko_enum_prefix="StyleBoxDirection",
                         animation_type="none",
                         alias="-webkit-box-direction",
                         spec="Nonstandard (https://developer.mozilla.org/en-US/docs/Web/CSS/box-direction)")}

${helpers.predefined_type("-moz-box-flex", "Number", "0.0", "parse_non_negative",
                          products="gecko", gecko_ffi_name="mBoxFlex",
                          animation_type="none",
                          alias="-webkit-box-flex",
                          spec="Nonstandard (https://developer.mozilla.org/en-US/docs/Web/CSS/box-flex)")}

${helpers.single_keyword("-moz-box-pack", "start center end justify",
                         products="gecko", gecko_ffi_name="mBoxPack",
                         gecko_enum_prefix="StyleBoxPack",
                         animation_type="none",
                         alias="-webkit-box-pack",
                         spec="Nonstandard (https://developer.mozilla.org/en-US/docs/Web/CSS/box-pack)")}

${helpers.single_keyword("-moz-stack-sizing", "stretch-to-fit ignore",
                         products="gecko", gecko_ffi_name="mStretchStack",
                         gecko_constant_prefix="NS_STYLE_STACK_SIZING",
                         animation_type="none",
                         spec="Nonstandard (https://developer.mozilla.org/en-US/docs/Web/CSS/-moz-stack-sizing)")}

${helpers.predefined_type("-moz-box-ordinal-group", "Integer", "0",
                          parse_method="parse_non_negative",
                          products="gecko",
                          alias="-webkit-box-ordinal-group",
                          gecko_ffi_name="mBoxOrdinal",
                          animation_type="none",
                          spec="Nonstandard (https://developer.mozilla.org/en-US/docs/Web/CSS/-moz-box-ordinal-group)")}

<%helpers:longhand name="-moz-box-orient"
                   products="gecko"
                   animation_type="none"
                   alias="-webkit-box-orient"
                   spec="Nonstandard (https://developer.mozilla.org/en-US/docs/Web/CSS/box-orient)">
    use style_traits::ToCss;
    use values::computed::ComputedValueAsSpecified;
    use values::HasViewportPercentage;

    define_css_keyword_enum!(BoxOrientKeyword:
                            "inline-axis" => InlineAxis,
                            "block-axis" => BlockAxis,
                            "horizontal" => Horizontal,
                            "vertical" => Vertical);

    pub use self::BoxOrientKeyword as SpecifiedValue;
    no_viewport_percentage!(SpecifiedValue);

    pub mod computed_value {
        pub use super::SpecifiedValue as T;
    }

    #[inline]
    pub fn get_initial_value() -> computed_value::T {
        self::computed_value::T::Horizontal
    }

    #[inline]
    pub fn parse(_context: &ParserContext, input: &mut Parser)
                 -> Result<SpecifiedValue, ()> {
        SpecifiedValue::parse(input)
    }

    impl Parse for SpecifiedValue {
        #[inline]
        fn parse(_context: &ParserContext, input: &mut Parser)
                 -> Result<SpecifiedValue, ()> {
            SpecifiedValue::parse(input)
        }
    }

    impl ComputedValueAsSpecified for SpecifiedValue {}
</%helpers:longhand>
