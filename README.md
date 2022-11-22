# Entropy Bot

Đỗ Ngọc Anh Trung & Nguyễn Đăng Dũng

Rules of Entropy: [click me !!!](https://www.codecup.nl/entropy/rules.php)

Made In C++

## To- do list

- Iterative Deepening
- Transpositional Table
- History Heuristic
- Killer Heuristic
- Aspiration Windows
- Principal Variation Search

Lưu ý rằng một số topics trên không cần phải được áp dụng luôn

## Notes for the above topics

- Iterative Deepening: ta tăng $depth = depth + 1$ mỗi lượt ($depth_{0} = 1$).
- Transpositional Table: ta lưu lại các vị trí của bàn mà có thể phải lặp đi lặp lại để giảm độ phức tạp (DP).
- History Heuristic: Ta sử dụng các vị trí mà $\alpha$ hoặc $\beta$ bị tạch ở bước này để trong lượt sau ta kiểm tra lại các vị trí đó. ([đọc thêm](https://www.chessprogramming.org/History_Heuristic))
- Killer Heuristic: Ta sử dụng các vị trí mà $\alpha$ hoặc $\beta$ bị tạch để sử dụng nó trong vị trí chị em (sibling node) (giả sử vị trí không thay đổi mấy) ([đọc thêm](https://www.chessprogramming.org/Killer_Heuristic))
- Aspiration Windows: Dựa trên alpha - beta pruning. Mình có thể giảm không gian prune alpha - beta của mình. Ta sử dụng một evaluation ở vòng lặp ở trước rồi dùng một window xung quanh giá trị này để làm $\alpha$ và $\beta$ ([đọc thêm](https://www.chessprogramming.org/Aspiration_Windows))
- Principal Variation Search: Dựa trên alpha - beta pruning. Ta kiểm tra trong khoảng [alpha - 1, alpha] đối với mỗi cách đi hợp lệ để xem có tệ hơn hay không.

## Some trivial notes

- cutoff là khi ta cách ly một cành của tree (ví dụ cutoff trong alpha-beta pruning xảy ra khi ta dừng tìm kiếm từ một vị trí tại vì $\beta <= \alpha$)
- Tạm thời bỏ qua neural network :D
