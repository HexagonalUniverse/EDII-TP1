mrtSeq :: Integral a => a -> [a]
mrtSeq n | n < 0 = map (\x -> - x) $ mrtSeq $ - n
mrtSeq n = __mrt 1 n [] where
    __mrt :: Integral a => a -> a -> [a] -> [a]
    __mrt i j xs
        | j >= i = m : __mrt i (m - 1) [] ++ __mrt (m + 1) j []
        | otherwise = xs
        where m = i + div (j - i) 2

