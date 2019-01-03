open Revery;
open Revery.Core;
open Revery.UI;
open Revery.UI.Components;

module Row = (
  val component((render, ~children, ()) =>
        render(
          () => {
            let style =
              Style.make(
                ~flexDirection=LayoutTypes.Row,
                ~alignItems=LayoutTypes.AlignStretch,
                ~justifyContent=LayoutTypes.JustifyCenter,
                ~flexGrow=1,
                (),
              );
            <view style> ...children </view>;
          },
          ~children,
        )
      )
);

module Column = (
  val component((render, ~children, ()) =>
        render(
          () => {
            let style =
              Style.make(
                ~flexDirection=LayoutTypes.Column,
                ~alignItems=LayoutTypes.AlignStretch,
                ~justifyContent=LayoutTypes.JustifyCenter,
                ~backgroundColor=Colors.darkGrey,
                ~flexGrow=1,
                (),
              );
            <view style> ...children </view>;
          },
          ~children,
        )
      )
);

module Button = (
  val component((render, ~contents: string, ~onClick, ~children, ()) =>
        render(
          () => {
            let viewStyle =
              Style.make(
                ~position=LayoutTypes.Relative,
                ~backgroundColor=Colors.lightGrey,
                ~justifyContent=LayoutTypes.JustifyCenter,
                ~alignItems=LayoutTypes.AlignCenter,
                /* TODO: Figure out relative sizing */
                ~width=150,
                ~height=120,
                ~flexGrow=1,
                ~margin=10,
                (),
              );
            let textStyle =
              Style.make(
                ~color=Colors.black,
                ~fontFamily="Roboto-Regular.ttf",
                ~fontSize=32,
                (),
              );

            <Clickable onClick>
              <view style=viewStyle>
                <text style=textStyle> contents </text>
              </view>
            </Clickable>;
          },
          ~children,
        )
      )
);

module Display = (
  val component((render, ~display: string, ~curNum: string, ~children, ()) =>
        render(
          () => {
            let viewStyle =
              Style.make(
                ~backgroundColor=Colors.white,
                ~height=120,
                ~flexDirection=LayoutTypes.Column,
                ~alignItems=LayoutTypes.AlignStretch,
                ~justifyContent=LayoutTypes.JustifyFlexEnd,
                ~flexGrow=2,
                (),
              );
            let displayStyle =
              Style.make(
                ~color=Colors.black,
                ~fontFamily="Roboto-Regular.ttf",
                ~fontSize=20,
                ~margin=15,
                (),
              );
            let numStyle =
              Style.make(
                ~color=Colors.black,
                ~fontFamily="Roboto-Regular.ttf",
                ~fontSize=32,
                ~margin=15,
                (),
              );

            <view style=viewStyle>
              <text style=displayStyle> display </text>
              <text style=numStyle> curNum </text>
            </view>;
          },
          ~children,
        )
      )
);

type operator = [ | `Nop | `Add | `Sub | `Mul | `Div];

let showFloat = float => {
  let string = string_of_float(float);
  if (String.length(string) > 1 && string.[String.length(string) - 1] == '.') {
    String.sub(string, 0, String.length(string) - 1);
  } else {
    string;
  };
};
type state = {
  operator, /* Current operator being applied */
  result: float, /* The actual numerical result */
  display: string, /* The equation displayed */
  number: string /* Current number being typed */
};

type action =
  | BackspaceKeyPressed
  | ClearKeyPressed(bool) /*AC pressed */
  | DotKeyPressed
  | NumberKeyPressed(string)
  | OperationKeyPressed(operator)
  | PlusMinusKeyPressed
  | ResultKeyPressed;

let eval = (state, newOp) => {
  /* Figure out what the string for the next operation will be */
  let newOpString =
    switch (newOp) {
    | `Nop => ""
    | `Add => "+"
    | `Sub => "-"
    | `Mul => "×"
    | `Div => "÷"
    };
  /* Split the current display on ! and get everything after (to clear errors) */
  let partitionedDisplay = String.split_on_char('!', state.display);
  let display =
    List.nth(partitionedDisplay, List.length(partitionedDisplay) - 1);
  let (newDisplay, newResult) =
    switch (state.operator) {
    | #operator when state.number == "" => (
        "Error: Can't evaluate binary operator without input!",
        state.result,
      )
    | `Nop => (state.number ++ newOpString, float_of_string(state.number))
    | `Add => (
        display ++ state.number ++ newOpString,
        state.result +. float_of_string(state.number),
      )
    | `Sub => (
        display ++ state.number ++ newOpString,
        state.result -. float_of_string(state.number),
      )
    | `Mul => (
        display ++ state.number ++ newOpString,
        state.result *. float_of_string(state.number),
      )
    | `Div =>
      if (float_of_string(state.number) != 0.) {
        (
          display ++ state.number ++ newOpString,
          state.result /. float_of_string(state.number),
        );
      } else {
        ("Error: Divide by zero!", state.result);
      }
    };
  (newResult, newDisplay);
};

let reducer = (state, action) =>
  switch (action) {
  | BackspaceKeyPressed =>
    state.number == "" ?
      state :
      {
        ...state,
        number: String.sub(state.number, 0, String.length(state.number) - 1),
      }
  | ClearKeyPressed(ac) =>
    ac ?
      {...state, operator: `Nop, result: 0., display: "", number: ""} :
      {...state, number: ""}
  | DotKeyPressed =>
    String.contains(state.number, '.') ?
      state : {...state, number: state.number ++ "."}
  | NumberKeyPressed(n) => {...state, number: state.number ++ n}
  | OperationKeyPressed(o) =>
    let (result, display) = eval(state, o);
    {operator: o, result, display, number: ""};
  | PlusMinusKeyPressed =>
    if (state.number != "" && state.number.[0] == '-') {
      {
        ...state,
        number: String.sub(state.number, 1, String.length(state.number) - 1),
      };
    } else {
      {...state, number: "-" ++ state.number};
    }
  | ResultKeyPressed =>
    let (result, display) = eval(state, `Nop);
    {operator: `Nop, result, display, number: showFloat(result)};
  };

module Calculator = (
  val component((render, ~children, ()) =>
        render(
          () => {
            let ({display, number, _}, dispatch) =
              useReducer(
                reducer,
                {operator: `Nop, result: 0., display: "", number: ""},
              );

            <Column>
              <Display display curNum=number />
              <Row>
                <Button
                  contents="AC"
                  onClick={_ => dispatch(ClearKeyPressed(true))}
                />
                <Button
                  contents="C"
                  onClick={_ => dispatch(ClearKeyPressed(false))}
                />
                <Button
                  contents="±"
                  onClick={_ => dispatch(PlusMinusKeyPressed)}
                />
                /* TODO: Switch to a font with a backspace character */
                <Button
                  contents="<="
                  onClick={_ => dispatch(BackspaceKeyPressed)}
                />
              </Row>
              <Row>
                <Button
                  contents="7"
                  onClick={_ => dispatch(NumberKeyPressed("7"))}
                />
                <Button
                  contents="8"
                  onClick={_ => dispatch(NumberKeyPressed("8"))}
                />
                <Button
                  contents="9"
                  onClick={_ => dispatch(NumberKeyPressed("9"))}
                />
                <Button
                  contents="÷"
                  onClick={_ => dispatch(OperationKeyPressed(`Div))}
                />
              </Row>
              <Row>
                <Button
                  contents="4"
                  onClick={_ => dispatch(NumberKeyPressed("4"))}
                />
                <Button
                  contents="5"
                  onClick={_ => dispatch(NumberKeyPressed("5"))}
                />
                <Button
                  contents="6"
                  onClick={_ => dispatch(NumberKeyPressed("6"))}
                />
                <Button
                  contents="×"
                  onClick={_ => dispatch(OperationKeyPressed(`Mul))}
                />
              </Row>
              <Row>
                <Button
                  contents="1"
                  onClick={_ => dispatch(NumberKeyPressed("1"))}
                />
                <Button
                  contents="2"
                  onClick={_ => dispatch(NumberKeyPressed("2"))}
                />
                <Button
                  contents="3"
                  onClick={_ => dispatch(NumberKeyPressed("3"))}
                />
                <Button
                  contents="-"
                  onClick={_ => dispatch(OperationKeyPressed(`Sub))}
                />
              </Row>
              <Row>
                <Button contents="." onClick={_ => dispatch(DotKeyPressed)} />
                <Button
                  contents="0"
                  onClick={_ => dispatch(NumberKeyPressed("0"))}
                />
                <Button
                  contents="="
                  onClick={_ => dispatch(ResultKeyPressed)}
                />
                <Button
                  contents="+"
                  onClick={_ => dispatch(OperationKeyPressed(`Add))}
                />
              </Row>
            </Column>;
          },
          ~children,
        )
      )
);

let init = app => {
  let window = App.createWindow(app, "Revery Calculator");

  let render = () => {
    <Calculator />;
  };

  UI.start(window, render);
};

App.start(init);