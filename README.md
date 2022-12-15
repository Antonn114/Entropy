# entropy bot

Đỗ Ngọc Anh Trung & Nguyễn Đăng Dũng

[click me !!!](https://www.codecup.nl/entropy/rules.php)

Made In C++

## algorithms

✅ Minimax

✅ Alpha - beta pruning

✅ Transpositional Table

✅ Principal Variation Search

🔵 Monte Carlo Tree Search

🔵 Expectiminimax

🔵 Aspiration Windows

🔵 Iterative Deepening

🔵 History Heuristic

## miscs

✅ Principal Variation Line

## strategies

- "Liberate the board" (for ORDER): in the beginning of the game, CHAOS would usually cluster the pieces in one spot (e.g: In the middle of the board, or on the first row). We can spread all these colours out in the different corners so that we can get a clear area to work with (either shooting them to other side of the board deliberately or other ways)

- "Control the middle" (for ORDER): We should favour in putting colours in the middle of the board (clustering the same colours in the same spot or creating palindromes in the middle of the board)

- "Gap in 3" (for ORDER): Create 3-palindromes by using 2 pieces of the same colour and a clear area (still counts as a valid palindrome) so that other colours could have a safe escape to the other side of the board.

## footnotes

- cutoff là khi ta cắt đi một cạnh của tree để giảm độ phức tạp thời gian và không gian (ví dụ cutoff trong alpha-beta pruning xảy ra khi ta dừng tìm kiếm từ một vị trí tại vì $\beta <= \alpha$)
- Tạm thời bỏ qua neural network :D
