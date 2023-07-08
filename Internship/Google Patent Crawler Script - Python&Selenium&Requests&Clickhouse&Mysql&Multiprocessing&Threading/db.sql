CREATE TABLE his_data_snaps.google_patent_data_common
(
    patent_id String Codec (ZSTD),
    title String Codec (ZSTD),
    assignee String Codec (ZSTD),
    inventor_author String Codec (ZSTD),
    priority_date Nullable(Date) Codec (ZSTD),
    filing_creation_date Date Codec (ZSTD),
    publication_date Nullable(Date) Codec (ZSTD),
    grant_date Nullable(Date) Codec (ZSTD),
    result_link Nullable(String) Codec (ZSTD),
    representative_figure_link Nullable(String) Codec (ZSTD),
    abstract Nullable(String) Codec (ZSTD),
    claims_num Nullable(UInt32) Codec (ZSTD),
    classifications Nullable(String) Codec (ZSTD),
    bopu_update_time DateTime64(6, 'UTC') default toDateTime64(toString(now64(6)), 6, 'UTC') Codec (ZSTD)
)
ENGINE = MergeTree
PARTITION BY toYYYYMM(filing_creation_date)
ORDER BY (filing_creation_date, patent_id)
SETTINGS index_granularity = 8192;


CREATE TABLE his_data_snaps.google_patent_data_cite
(
    patent_id String Codec (ZSTD),
    cited_by_patent_id String Codec (ZSTD),
    priority_date Date Codec (ZSTD),
    publication_date Date Codec (ZSTD),
    assignee String Codec (ZSTD),
    title String Codec (ZSTD),
    bopu_update_time DateTime64(6, 'UTC') default toDateTime64(toString(now64(6)), 6, 'UTC') Codec (ZSTD)
)
ENGINE = MergeTree
PARTITION BY sipHash64(patent_id) % 16
ORDER BY (patent_id, cited_by_patent_id)
SETTINGS index_granularity = 8192;
