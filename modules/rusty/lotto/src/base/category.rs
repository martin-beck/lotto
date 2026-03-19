use lotto_sys as raw;

pub type Category = raw::base_category;

pub fn effective_event_type(ctx: &raw::context_t) -> u32 {
    ctx.type_ as u32
}

pub fn effective_category(ctx: &raw::context_t) -> Category {
    match effective_event_type(ctx) {
        raw::EVENT_TASK_CREATE => Category::CAT_TASK_CREATE,
        raw::EVENT_CALL => Category::CAT_CALL,
        raw::EVENT_TASK_INIT => Category::CAT_TASK_INIT,
        raw::EVENT_TASK_FINI => Category::CAT_TASK_FINI,
        raw::EVENT_TASK_DETACH => Category::CAT_DETACH,
        raw::EVENT_KEY_CREATE => Category::CAT_KEY_CREATE,
        raw::EVENT_KEY_DELETE => Category::CAT_KEY_DELETE,
        raw::EVENT_SET_SPECIFIC => Category::CAT_SET_SPECIFIC,
        raw::EVENT_BEFORE_READ => Category::CAT_BEFORE_READ,
        raw::EVENT_BEFORE_WRITE => Category::CAT_BEFORE_WRITE,
        raw::EVENT_BEFORE_AREAD => Category::CAT_BEFORE_AREAD,
        raw::EVENT_BEFORE_AWRITE => Category::CAT_BEFORE_AWRITE,
        raw::EVENT_BEFORE_RMW => Category::CAT_BEFORE_RMW,
        raw::EVENT_BEFORE_XCHG => Category::CAT_BEFORE_XCHG,
        raw::EVENT_BEFORE_CMPXCHG => Category::CAT_BEFORE_CMPXCHG,
        raw::EVENT_BEFORE_FENCE => Category::CAT_BEFORE_FENCE,
        raw::EVENT_AFTER_AREAD => Category::CAT_AFTER_AREAD,
        raw::EVENT_AFTER_AWRITE => Category::CAT_AFTER_AWRITE,
        raw::EVENT_AFTER_RMW => Category::CAT_AFTER_RMW,
        raw::EVENT_AFTER_XCHG => Category::CAT_AFTER_XCHG,
        raw::EVENT_AFTER_CMPXCHG_S => Category::CAT_AFTER_CMPXCHG_S,
        raw::EVENT_AFTER_CMPXCHG_F => Category::CAT_AFTER_CMPXCHG_F,
        raw::EVENT_AFTER_FENCE => Category::CAT_AFTER_FENCE,
        raw::EVENT_FUNC_ENTRY => Category::CAT_FUNC_ENTRY,
        raw::EVENT_FUNC_EXIT => Category::CAT_FUNC_EXIT,
        _ => ctx.cat,
    }
}
