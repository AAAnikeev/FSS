--
-- PostgreSQL database dump
--

-- Dumped from database version 9.6.6
-- Dumped by pg_dump version 9.6.6

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET row_security = off;

DROP DATABASE file_share;
--
-- Name: file_share; Type: DATABASE; Schema: -; Owner: postgres
--

CREATE DATABASE file_share WITH TEMPLATE = template0 ENCODING = 'UTF8' LC_COLLATE = 'ru_RU.UTF-8' LC_CTYPE = 'ru_RU.UTF-8';


ALTER DATABASE file_share OWNER TO postgres;

\connect file_share

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET row_security = off;

--
-- Name: DATABASE file_share; Type: MAC LABEL; Schema: -; Owner: postgres
--

MAC LABEL ON DATABASE CURRENT_CATALOG IS '{0,0}';


--
-- Name: DATABASE file_share; Type: MAC CCR; Schema: -; Owner: postgres
--

MAC CCR ON DATABASE CURRENT_CATALOG IS ON;


--
-- Name: SCHEMA public; Type: MAC LABEL; Schema: -; Owner: postgres
--

MAC LABEL ON SCHEMA public IS '{0,0}';


--
-- Name: SCHEMA public; Type: MAC CCR; Schema: -; Owner: postgres
--

MAC CCR ON SCHEMA public IS ON;


--
-- Name: plpgsql; Type: EXTENSION; Schema: -; Owner: 
--

CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;


--
-- Name: EXTENSION plpgsql; Type: COMMENT; Schema: -; Owner: 
--

COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';


SET search_path = pg_catalog;

--
-- Name: EXTENSION plpgsql; Type: MAC LABEL; Schema: pg_catalog; Owner: 
--

MAC LABEL ON EXTENSION plpgsql IS '{0,0}';


--
-- Name: LANGUAGE plpgsql; Type: MAC LABEL; Schema: -; Owner: 
--

MAC LABEL ON LANGUAGE plpgsql IS '{0,0}';


SET search_path = public, pg_catalog;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: href_file; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE href_file (
    group_id character varying(64) NOT NULL,
    file_id integer NOT NULL,
    file_path character varying(500),
    file_name character varying(500)
)
WITH (MACS=FALSE);


ALTER TABLE href_file OWNER TO postgres;

--
-- Name: TABLE href_file; Type: MAC LABEL; Schema: public; Owner: postgres
--

MAC LABEL ON TABLE href_file IS '{0,0}';


--
-- Name: TABLE href_file; Type: MAC CCR; Schema: public; Owner: postgres
--

MAC CCR ON TABLE href_file IS ON;


--
-- Data for Name: href_file; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY href_file (group_id, file_id, file_path, file_name) FROM stdin;
\.


-- Obtained maclabel {0,0}
--
-- Name: href_file file_id_pk; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY href_file
    ADD CONSTRAINT file_id_pk PRIMARY KEY (group_id, file_id);


--
-- Name: public; Type: ACL; Schema: -; Owner: postgres
--

GRANT ALL ON SCHEMA public TO PUBLIC;
GRANT ALL ON SCHEMA public TO fss;


--
-- PostgreSQL database dump complete
--

