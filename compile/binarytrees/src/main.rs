use std::thread;
use typed_arena::Arena;

struct Tree<'a> {
    l: Option<&'a Tree<'a>>,
    r: Option<&'a Tree<'a>>,
}

fn item_check(tree: &Option<&Tree<'_>>) -> i32 {
    match tree {
        None => 0,
        Some(node) => 1 + item_check(&node.l) + item_check(&node.r),
    }
}

fn bottom_up_tree<'a>(
    arena: &'a Arena<Tree<'a>>,
    depth: i32,
) -> Option<&'a Tree<'a>> {
    if depth >= 0 {
        Some(arena.alloc(Tree {
            l: bottom_up_tree(arena, depth - 1),
            r: bottom_up_tree(arena, depth - 1),
        }))
    } else {
        None
    }
}

fn inner(depth: i32, iterations: i32) -> String {
    let mut chk = 0;

    for _ in 0..iterations {
        let arena = Arena::new();
        let tree = bottom_up_tree(&arena, depth);
        chk += item_check(&tree);
    }

    format!(
        "{}\t trees of depth {}\t check: {}",
        iterations, depth, chk
    )
}

fn main() {
    let n: i32 = std::env::args()
        .nth(1)
        .and_then(|s| s.parse().ok())
        .unwrap_or(10);

    let min_depth = 4;
    let max_depth = std::cmp::max(min_depth + 2, n);

    {
        let arena = Arena::new();
        let depth = max_depth + 1;
        let tree = bottom_up_tree(&arena, depth);

        println!(
            "stretch tree of depth {}\t check: {}",
            depth,
            item_check(&tree)
        );
    }

    let long_lived_arena = Arena::new();
    let long_lived_tree = bottom_up_tree(&long_lived_arena, max_depth);

    let messages: Vec<_> = (min_depth..=max_depth)
        .filter(|d| d % 2 == 0)
        .map(|depth| {
            let iterations = 1 << ((max_depth - depth + min_depth) as u32);
            thread::spawn(move || inner(depth, iterations))
        })
        .collect();

    for message in messages {
        println!("{}", message.join().unwrap());
    }

    println!(
        "long lived tree of depth {}\t check: {}",
        max_depth,
        item_check(&long_lived_tree)
    );
}
