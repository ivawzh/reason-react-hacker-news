open Utils;

requireCSS("src/CommentList.css");

type action =
  | Toggle(option(string));

type state = {collapsed_comments: JSSet.set(int)};

let component = ReasonReact.reducerComponent("CommentList");
let make = (~story: StoryData.story_with_comments, _children) => {
  let toggleComment = (collapsed, idMaybe: option(string)) =>
    switch idMaybe {
    | Some(idString) =>
      let id = int_of_string(idString);
      if (JSSet.has(collapsed, id)) {
        JSSet.remove(collapsed, id)
      } else {
        JSSet.add(collapsed, id)
      }
    | None => collapsed
    };

  let getCommentIdFromEvent = (event: ReactEventRe.Mouse.t) =>
    getAttribute(ReactDOMRe.domElementToObj(ReactEventRe.Mouse.currentTarget(event)), "name");

  let renderCommentText = (textMaybe: option(string)) =>
    switch textMaybe {
    | Some(text) => <div dangerouslySetInnerHTML=(dangerousHtml(text)) />
    | None => textEl("missing comment")
    };

  let rec renderCommentKids = (self, comment: StoryData.comment_present) =>
    renderCommentList(self, comment.kids)

  and renderComment = ({ ReasonReact.state } as self, id: int) => {
    let commentMaybe = JSMap.get(story.comments, id);

    <div key=(string_of_int(id))>
      (
        switch commentMaybe {
        | Some(commentPresentOrDeleted) =>
          switch commentPresentOrDeleted {

          | StoryData.CommentPresent(comment) =>
            let openComment = ! JSSet.has(state.collapsed_comments, comment.id);
            <div className="CommentList_comment">

              <div className="CommentList_disclosureRow CommentList_inline"
                   name=(string_of_int(comment.id))
                   onClick=(self.reduce(event => Toggle(getCommentIdFromEvent(event))))>

                <img alt=(openComment ? "hide" : "show")
                     src=(
                       openComment ?
                         requireAssetURI("src/disclosure90.png") :
                         requireAssetURI("src/disclosure.png")
                     )
                     className="CommentList_disclosure CommentList_muted"
                />

                <span className="CommentList_muted">
                  ({
                    let time = fromNow(comment.time);
                    let by = comment.by;
                    textEl({j| $time by $by|j})
                  })
                </span>

              </div>

              (
                if (openComment) {
                  <div className="CommentList_commentBody">
                    (renderCommentText(comment.text))
                    (renderCommentKids(self, comment))
                  </div>
                } else {
                  <noscript />
                }
              )
            </div>

          | StoryData.CommentDeleted(_) =>
            <div className="CommentList_error"> (textEl("[comment deleted (id="++string_of_int(id)++")]")) </div>
          }

        | None => <div className="CommentList_error"> (textEl("[comment not loaded (id="++string_of_int(id)++")]")) </div>
        }
      )
    </div>
  }

  and renderCommentList = (self, commentIds: option(array(int))) =>
    switch commentIds {
    | Some(ids) =>
      let commentList = Array.map(renderComment(self), ids);
      <div> (arrayEl(commentList)) </div>
    | None => <div />
    };

  {
    ...component,

    initialState: () => {
      collapsed_comments: JSSet.create([||]: array(int))
    },

    reducer: (action, state) =>
      switch action {
      | Toggle(commentId) =>
        ReasonReact.Update({
          collapsed_comments: toggleComment(state.collapsed_comments, commentId)
        })
      },

    render: (self) =>
      renderCommentList(self, story.kids)
  }
};
