const express = require("express");
const { Pool } = require("pg");

const app = express();
const port = 3000;

app.use(express.json());


const pool = new Pool({
  user: "postgres",
  host: "localhost",
  database: "postgres",
  password: "123321",
  port: 5432,
});


pool.connect()
  .then(() => console.log("Database connected successfully"))
  .catch(err => console.error("Database connection error:", err));


app.get("/get_coffee_data", async (req, res) => {
  const { card_id } = req.query;

  if (!card_id) {
    return res.status(400).json({ error: "Card ID required!" });
  }

  try {
    const result = await pool.query(
      "SELECT coffee_name FROM customer WHERE card_id ILIKE $1",
      [card_id]
    );

    if (result.rows.length > 0) {
      res.json({ coffee_name: result.rows[0].coffee_name });
    } else {
      res.status(404).json({ error: "Card ID not found" });
    }
  } catch (err) {
    console.error("Database Error:", err);
    res.status(500).json({ error: "Database Error" });
  }
});


// Sunucuyu başlat
app.listen(port, () => {
  console.log(`Server running on http://localhost:${port}`);
});
