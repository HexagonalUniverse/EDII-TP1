


data Tree a = EmptyTree | Root a (Tree a) (Tree a)

instance Show a => Show (Tree a) where
    show :: (Show a) => Tree a -> String
    show EmptyTree = "None"

    show (Root x l r) = "<" ++ show x ++ ", " ++ show l ++ ", " ++ show r ++ ">"


middlePoint :: [a] -> Int
middlePoint list
    | even l_length = (div l_length 2) - 1
    | otherwise = div l_length 2 
    where
        l_length = length list


leftHalf :: [a] -> [a]
leftHalf list = take (middlePoint list) list

rightHalf :: [a] -> [a]
rightHalf list = drop (middlePoint list + 1) list


mrt :: [a] -> Tree a
mrt = traverse where
    traverse :: [a] -> Tree a
    traverse [] = EmptyTree
    traverse list = Root root left_half right_half where
        root = list !! middle

        middle :: Int
        -- middle = div (length list) 2
        middle = middlePoint list

        left_half = traverse $ take middle list
        right_half = traverse $ drop (middle + 1) list



    